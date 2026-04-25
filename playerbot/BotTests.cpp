#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "BotTests.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <deque>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <sys/stat.h>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#endif

using namespace ai;

namespace
{
    const char* TEST_RESULTS_FILE = "bot_test_results.log";
    const char* HISTORY_FILE = "bot_test_summary_history.csv";
    const char* LANDING_PAGE_FILE = "test_status.html";
    const char* REPORT_SUBDIR = "test_report";
    const char* REPORT_TESTS_SUBDIR = "tests";

    struct TestRecord
    {
        std::string timestamp;
        std::string botName;
        std::string testName;
        std::string result;
        std::string message;
    };

    struct TestResultCounts
    {
        uint32 pass = 0;
        uint32 fail = 0;
        uint32 abortCount = 0;
        uint32 impossible = 0;
    };

    struct TestTreeNode
    {
        std::string name;
        std::string fullName;
        TestResultCounts counts;
        bool hasChildren = false;
        bool isExpanded = false;
        std::vector<TestTreeNode*> children;
        TestTreeNode* parent = nullptr;
    };

    struct TestSummary
    {
        std::string generatedAt;
        uint32 total = 0;
        uint32 pass = 0;
        uint32 fail = 0;
        uint32 abortCount = 0;
        uint32 impossible = 0;
        std::map<std::string, uint32> failingTests;
        std::map<std::string, std::vector<TestRecord>> failingRecords;
        std::map<std::string, TestResultCounts> allTestCounts;
        std::vector<TestRecord> allRecords;
    };

    struct HistoryRow
    {
        std::string timestamp;
        uint32 total = 0;
        uint32 pass = 0;
        uint32 fail = 0;
        uint32 abortCount = 0;
        uint32 impossible = 0;
        std::string passRate;
        std::string topFailingTest;
    };

    std::string Trim(const std::string& value)
    {
        size_t begin = 0;
        while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])))
            ++begin;

        size_t end = value.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])))
            --end;

        return value.substr(begin, end - begin);
    }

    std::string ToUpperCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return value;
    }

    std::string GetLogsDir()
    {
        std::string logsDir = sConfig.GetStringDefault("LogsDir");
        if (!logsDir.empty() && logsDir.back() != '/' && logsDir.back() != '\\')
            logsDir.push_back('/');

        return logsDir;
    }

    std::string JoinPath(const std::string& base, const std::string& relative)
    {
        if (base.empty())
            return relative;

        if (base.back() == '/' || base.back() == '\\')
            return base + relative;

        return base + "/" + relative;
    }

    bool EnsureDir(const std::string& path)
    {
        if (path.empty())
            return false;

#ifdef _WIN32
        int result = _mkdir(path.c_str());
#else
        int result = mkdir(path.c_str(), 0755);
#endif
        return result == 0 || errno == EEXIST;
    }

    std::string HtmlEscape(const std::string& input)
    {
        std::string output;
        output.reserve(input.size());

        for (char c : input)
        {
            switch (c)
            {
                case '&': output += "&amp;"; break;
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '"': output += "&quot;"; break;
                default: output += c; break;
            }
        }

        return output;
    }

    std::string SanitizeFileName(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
        {
            if (std::isalnum(c) || c == '-' || c == '_')
                return static_cast<char>(c);

            return '_';
        });

        if (value.empty())
            return "unnamed_test";

        return value;
    }

    std::vector<std::string> SplitBy(const std::string& input, const std::string& delimiter)
    {
        std::vector<std::string> result;
        if (delimiter.empty())
            return result;

        size_t pos = 0;
        while (pos <= input.size())
        {
            size_t next = input.find(delimiter, pos);
            if (next == std::string::npos)
            {
                result.push_back(input.substr(pos));
                break;
            }

            result.push_back(input.substr(pos, next - pos));
            pos = next + delimiter.size();
        }

        return result;
    }

    bool ParseTestRecord(const std::string& line, TestRecord& record)
    {
        const std::string prefix = "[BOTTEST] ";
        if (line.find(prefix) != 0)
            return false;

        std::vector<std::string> parts = SplitBy(line.substr(prefix.size()), " | ");
        if (parts.size() < 5)
            return false;

        record.timestamp = Trim(parts[0]);
        record.botName = Trim(parts[1]);
        record.testName = Trim(parts[2]);
        record.result = ToUpperCopy(Trim(parts[3]));

        std::ostringstream message;
        for (size_t i = 4; i < parts.size(); ++i)
        {
            if (i > 4)
                message << " | ";
            message << parts[i];
        }

        record.message = Trim(message.str());
        return !record.timestamp.empty() && !record.testName.empty();
    }

    std::string FormatPassRate(uint32 pass, uint32 total)
    {
        if (!total)
            return "0.00%";

        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << ((100.0 * pass) / static_cast<double>(total)) << "%";
        return out.str();
    }

    std::string TopFailingTest(const std::map<std::string, uint32>& failingTests)
    {
        if (failingTests.empty())
            return "none";

        auto best = std::max_element(failingTests.begin(), failingTests.end(), [](const std::pair<std::string, uint32>& left, const std::pair<std::string, uint32>& right)
        {
            if (left.second != right.second)
                return left.second < right.second;

            return left.first > right.first;
        });

        return best->first;
    }

    std::vector<std::string> ReadTailLines(const std::string& filePath, size_t maxLines)
    {
        std::vector<std::string> lines;
        std::ifstream in(filePath.c_str(), std::ifstream::in);
        if (in.fail())
            return lines;

        std::deque<std::string> tail;
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;

            if (tail.size() == maxLines)
                tail.pop_front();

            tail.push_back(line);
        }

        lines.assign(tail.begin(), tail.end());
        return lines;
    }

    std::vector<HistoryRow> LoadHistoryRows(const std::string& filePath)
    {
        std::vector<HistoryRow> rows;
        std::ifstream in(filePath.c_str(), std::ifstream::in);
        if (in.fail())
            return rows;

        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty() || line.find("Timestamp,") == 0)
                continue;

            std::vector<std::string> tokens = SplitBy(line, ",");
            if (tokens.size() < 8)
                continue;

            try
            {
                HistoryRow row;
                row.timestamp = Trim(tokens[0]);
                row.total = static_cast<uint32>(std::stoi(tokens[1]));
                row.pass = static_cast<uint32>(std::stoi(tokens[2]));
                row.fail = static_cast<uint32>(std::stoi(tokens[3]));
                row.abortCount = static_cast<uint32>(std::stoi(tokens[4]));
                row.impossible = static_cast<uint32>(std::stoi(tokens[5]));
                row.passRate = Trim(tokens[6]);
                row.topFailingTest = Trim(tokens[7]);
                rows.push_back(row);
            }
            catch (...)
            {
                continue;
            }
        }

        return rows;
    }

    void AppendHistoryRow(const std::string& filePath, const HistoryRow& row)
    {
        bool writeHeader = true;
        {
            std::ifstream check(filePath.c_str(), std::ifstream::in);
            writeHeader = check.fail() || check.peek() == std::ifstream::traits_type::eof();
        }

        std::ofstream out(filePath.c_str(), std::ios::app);
        if (!out.is_open())
            return;

        if (writeHeader)
            out << "Timestamp,Total,Pass,Fail,Abort,Impossible,PassRate,TopFailingTest\n";

        std::string topFailing = row.topFailingTest;
        std::replace(topFailing.begin(), topFailing.end(), ',', ';');

        out << row.timestamp << ","
            << row.total << ","
            << row.pass << ","
            << row.fail << ","
            << row.abortCount << ","
            << row.impossible << ","
            << row.passRate << ","
            << topFailing << "\n";
    }

    void WriteTextFile(const std::string& filePath, const std::string& content)
    {
        std::ofstream out(filePath.c_str(), std::ios::out | std::ios::trunc);
        if (!out.is_open())
            return;

        out << content;
    }

    std::string GetResultSphereClass(const TestResultCounts& counts)
    {
        if (counts.fail > 0)
            return "sphere-red";
        return "sphere-green";
    }

    std::string HtmlNodeRow(TestTreeNode* node, uint8_t indentLevel, bool isLastChild)
    {
        std::ostringstream row;
        std::string leafClass = node->hasChildren ? "node-branch" : "node-leaf";
        std::string sphereClass = GetResultSphereClass(node->counts);

        row << "<div class=\"tree-row\">"
            << "<span class=\"tree-indent\" style=\"width:" << (indentLevel * 20) << "px\"></span>";

        if (node->hasChildren)
        {
            row << "<span class=\"toggle-btn\" onclick=\"toggleNode(this)\">"
                << "<span class=\"toggle-icon\">" << (node->isExpanded ? "&#9660;" : "&#9654;") << "</span> "
                << "</span>";
        }
        else
        {
            row << "<span class=\"toggle-spacer\"></span>";
        }

        row << "<span class=\"sphere " << sphereClass << "\"></span>"
            << "<span class=\"node-label " << leafClass << "\">" << HtmlEscape(node->name) << "</span>"
            << "<span class=\"node-counts\">"
            << "<span class=\"count-imp\" title=\"Impossible\">" << node->counts.impossible << "</span>"
            << "<span class=\"count-fail\" title=\"Failed\">" << node->counts.fail << "</span>"
            << "<span class=\"count-pass\" title=\"Passed\">" << node->counts.pass << "</span>"
            << "<span class=\"count-abort\" title=\"Aborted\">" << node->counts.abortCount << "</span>"
            << "</span></div>";

        if (node->hasChildren)
        {
            row << "<div class=\"tree-children\" style=\"display:" << (node->isExpanded ? "block" : "none") << "\">";
            for (TestTreeNode* child : node->children)
            {
                row << HtmlNodeRow(child, indentLevel + 1, child == node->children.back());
            }
            row << "</div>";
        }

        return row.str();
    }

    std::vector<TestTreeNode*> BuildTestTree(const std::vector<TestRecord>& records)
    {
        std::map<std::string, TestTreeNode*> nodeMap;
        std::vector<TestTreeNode*> rootNodes;

        for (const TestRecord& record : records)
        {
            const std::string& testName = record.testName;
            std::vector<std::string> parts = SplitBy(testName, "_");

            std::string prefix = parts.empty() ? testName : parts[0];
            std::string currentPath = prefix;

            if (nodeMap.find(currentPath) == nodeMap.end())
            {
                TestTreeNode* node = new TestTreeNode();
                node->name = prefix;
                node->fullName = prefix;
                node->parent = nullptr;
                nodeMap[currentPath] = node;
                rootNodes.push_back(node);
            }

            TestTreeNode* parentNode = nodeMap[currentPath];

            for (size_t i = 1; i < parts.size(); ++i)
            {
                currentPath += "_" + parts[i];

                if (nodeMap.find(currentPath) == nodeMap.end())
                {
                    TestTreeNode* node = new TestTreeNode();
                    node->name = parts[i];
                    node->fullName = currentPath;
                    node->parent = parentNode;
                    nodeMap[currentPath] = node;
                    parentNode->children.push_back(node);
                    parentNode->hasChildren = true;
                }

                parentNode = nodeMap[currentPath];
            }

            parentNode->counts.pass += (record.result == "PASS") ? 1 : 0;
            parentNode->counts.fail += (record.result == "FAIL") ? 1 : 0;
            parentNode->counts.abortCount += (record.result == "ABORT") ? 1 : 0;
            parentNode->counts.impossible += (record.result == "IMPOSSIBLE") ? 1 : 0;

            TestTreeNode* countNode = parentNode;
            while (countNode->parent)
            {
                countNode = countNode->parent;
                countNode->counts.pass += (record.result == "PASS") ? 1 : 0;
                countNode->counts.fail += (record.result == "FAIL") ? 1 : 0;
                countNode->counts.abortCount += (record.result == "ABORT") ? 1 : 0;
                countNode->counts.impossible += (record.result == "IMPOSSIBLE") ? 1 : 0;
            }
        }

        for (TestTreeNode* root : rootNodes)
        {
            root->isExpanded = true;
        }

        return rootNodes;
    }

    void PropagateCountsToParents(TestTreeNode* node)
    {
        TestTreeNode* parent = node->parent;
        while (parent)
        {
            parent->counts.pass += node->counts.pass;
            parent->counts.fail += node->counts.fail;
            parent->counts.abortCount += node->counts.abortCount;
            parent->counts.impossible += node->counts.impossible;
            parent = parent->parent;
        }
    }

    void DeleteTestTree(std::vector<TestTreeNode*>& roots)
    {
        std::set<TestTreeNode*> toDelete;
        std::function<void(TestTreeNode*)> collect = [&](TestTreeNode* node)
        {
            for (TestTreeNode* child : node->children)
                collect(child);
            toDelete.insert(node);
        };

        for (TestTreeNode* root : roots)
            collect(root);

        for (TestTreeNode* node : toDelete)
            delete node;
    }

    std::string GenerateTestTreeHtml(const std::vector<TestTreeNode*>& roots)
    {
        std::ostringstream html;
        html << "<div class=\"test-tree\">";
        for (TestTreeNode* root : roots)
        {
            html << HtmlNodeRow(root, 0, root == roots.back());
        }
        html << "</div>";
        return html.str();
    }

    void GenerateReportSite(const std::string& logsDir, const TestSummary& summary, const std::vector<HistoryRow>& historyRows)
    {
        const std::string reportDir = JoinPath(logsDir, REPORT_SUBDIR);
        const std::string testsDir = JoinPath(reportDir, REPORT_TESTS_SUBDIR);

        EnsureDir(reportDir);
        EnsureDir(testsDir);

        std::vector<std::pair<std::string, uint32>> failingTests;
        for (const auto& entry : summary.failingTests)
            failingTests.push_back(entry);

        std::sort(failingTests.begin(), failingTests.end(), [](const std::pair<std::string, uint32>& left, const std::pair<std::string, uint32>& right)
        {
            if (left.second != right.second)
                return left.second > right.second;

            return left.first < right.first;
        });

        const std::string nav = "<nav><a href=\"overview.html\">Overview</a> | <a href=\"tests.html\">Tests</a> | <a href=\"failures.html\">Failures</a> | <a href=\"history.html\">History</a></nav>";

        std::ostringstream overview;
        overview << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Bot Test Overview</title>"
                 << "<style>body{font-family:Segoe UI,Arial,sans-serif;margin:24px;}"
                 << "nav{margin-bottom:16px;} .card{display:inline-block;min-width:140px;padding:12px;margin:6px;border:1px solid #bbb;border-radius:8px;}"
                 << "h1,h2{margin:8px 0;} table{border-collapse:collapse;margin-top:12px;} th,td{border:1px solid #bbb;padding:6px 10px;text-align:left;}"
                 << "pre{background:#f4f4f4;border:1px solid #ccc;padding:10px;border-radius:6px;overflow:auto;}"
                 << "</style></head><body>"
                 << nav
                 << "<h1>Bot Test Overview</h1>"
                 << "<p>Generated at: " << HtmlEscape(summary.generatedAt) << "</p>"
                 << "<div class=\"card\"><strong>Total</strong><br>" << summary.total << "</div>"
                 << "<div class=\"card\"><strong>PASS</strong><br>" << summary.pass << "</div>"
                 << "<div class=\"card\"><strong>FAIL</strong><br>" << summary.fail << "</div>"
                 << "<div class=\"card\"><strong>ABORT</strong><br>" << summary.abortCount << "</div>"
                 << "<div class=\"card\"><strong>IMPOSSIBLE</strong><br>" << summary.impossible << "</div>"
                 << "<div class=\"card\"><strong>Pass rate</strong><br>" << HtmlEscape(FormatPassRate(summary.pass, summary.total)) << "</div>";

        if (!historyRows.empty())
        {
            const HistoryRow& current = historyRows.back();
            overview << "<h2>Current boot</h2>"
                     << "<p>Status: " << (current.fail == 0 && current.abortCount == 0 && current.impossible == 0 ? "Healthy" : "Issues detected") << "</p>";

            if (historyRows.size() > 1)
            {
                const HistoryRow& previous = historyRows[historyRows.size() - 2];
                overview << "<h2>Last boot</h2>"
                         << "<p>Timestamp: " << HtmlEscape(previous.timestamp)
                         << " | Total: " << previous.total
                         << " | Pass rate: " << HtmlEscape(previous.passRate)
                         << " | Top failing test: " << HtmlEscape(previous.topFailingTest)
                         << "</p>";
            }
        }

        std::vector<std::string> latestAnalysis = ReadTailLines(JoinPath(logsDir, "log_analysis.csv"), 8);
        if (!latestAnalysis.empty())
        {
            overview << "<h2>Latest log_analysis.csv rows</h2><pre>";
            for (const std::string& line : latestAnalysis)
                overview << HtmlEscape(line) << "\n";
            overview << "</pre>";
        }

        overview << "</body></html>";
        WriteTextFile(JoinPath(reportDir, "overview.html"), overview.str());

        std::ostringstream failures;
        failures << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Bot Test Failures</title>"
                 << "<style>body{font-family:Segoe UI,Arial,sans-serif;margin:24px;} ul{line-height:1.7;} nav{margin-bottom:16px;}</style>"
                 << "</head><body>" << nav << "<h1>Failed Test Drilldown</h1>";

        if (failingTests.empty())
        {
            failures << "<p>No failing tests captured for this boot.</p>";
        }
        else
        {
            failures << "<ul>";
            for (const auto& [testName, count] : failingTests)
            {
                const std::string fileName = SanitizeFileName(testName) + ".html";
                failures << "<li><a href=\"tests/" << HtmlEscape(fileName) << "\">" << HtmlEscape(testName) << "</a> (" << count << ")</li>";

                auto recordsIt = summary.failingRecords.find(testName);
                if (recordsIt == summary.failingRecords.end())
                    continue;

                std::ostringstream detail;
                detail << "<!doctype html><html><head><meta charset=\"utf-8\"><title>" << HtmlEscape(testName) << "</title>"
                       << "<style>body{font-family:Segoe UI,Arial,sans-serif;margin:24px;} table{border-collapse:collapse;}"
                       << "th,td{border:1px solid #bbb;padding:6px 10px;text-align:left;} nav{margin-bottom:16px;}"
                       << "</style></head><body>"
                       << "<nav><a href=\"../overview.html\">Overview</a> | <a href=\"../tests.html\">Tests</a> | <a href=\"../failures.html\">Failures</a> | <a href=\"../history.html\">History</a></nav>"
                       << "<h1>" << HtmlEscape(testName) << "</h1>"
                       << "<p>Failure count: " << count << "</p>"
                       << "<table><tr><th>Timestamp</th><th>Bot</th><th>Result</th><th>Message</th></tr>";

                for (const TestRecord& record : recordsIt->second)
                {
                    detail << "<tr><td>" << HtmlEscape(record.timestamp)
                           << "</td><td>" << HtmlEscape(record.botName)
                           << "</td><td>" << HtmlEscape(record.result)
                           << "</td><td>" << HtmlEscape(record.message)
                           << "</td></tr>";
                }

                detail << "</table></body></html>";
                WriteTextFile(JoinPath(testsDir, fileName), detail.str());
            }
            failures << "</ul>";
        }

        failures << "</body></html>";
        WriteTextFile(JoinPath(reportDir, "failures.html"), failures.str());

        std::ostringstream history;
        history << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Bot Test History</title>"
                << "<style>body{font-family:Segoe UI,Arial,sans-serif;margin:24px;} table{border-collapse:collapse;}"
                << "th,td{border:1px solid #bbb;padding:6px 10px;text-align:left;} nav{margin-bottom:16px;}"
                << "</style></head><body>" << nav << "<h1>Boot History</h1>"
                << "<table><tr><th>Timestamp</th><th>Total</th><th>Pass</th><th>Fail</th><th>Abort</th><th>Impossible</th><th>Pass rate</th><th>Top failing test</th></tr>";

        size_t start = 0;
        if (historyRows.size() > 50)
            start = historyRows.size() - 50;

        for (size_t i = start; i < historyRows.size(); ++i)
        {
            const HistoryRow& row = historyRows[i];
            history << "<tr><td>" << HtmlEscape(row.timestamp)
                    << "</td><td>" << row.total
                    << "</td><td>" << row.pass
                    << "</td><td>" << row.fail
                    << "</td><td>" << row.abortCount
                    << "</td><td>" << row.impossible
                    << "</td><td>" << HtmlEscape(row.passRate)
                    << "</td><td>" << HtmlEscape(row.topFailingTest)
                    << "</td></tr>";
        }

        history << "</table></body></html>";
        WriteTextFile(JoinPath(reportDir, "history.html"), history.str());

        std::ostringstream testsPage;
        testsPage << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Bot Test Tree</title>"
                  << "<style>"
                  << "body{font-family:Segoe UI,Arial,sans-serif;margin:24px;}"
                  << "nav{margin-bottom:16px;}"
                  << ".test-tree{margin-top:12px;font-size:14px;}"
                  << ".tree-row{display:flex;align-items:center;padding:4px 8px;border-radius:4px;}"
                  << ".tree-row:hover{background:#f0f0f0;}"
                  << ".tree-indent{display:inline-block;}"
                  << ".toggle-btn{cursor:pointer;width:20px;display:inline-flex;align-items:center;justify-content:center;color:#555;}"
                  << ".toggle-spacer{width:20px;display:inline-block;}"
                  << ".toggle-icon{font-size:10px;}"
                  << ".sphere{width:12px;height:12px;border-radius:50%;display:inline-block;margin:0 8px;flex-shrink:0;}"
                  << ".sphere-green{background:#22c55e;}"
                  << ".sphere-red{background:#ef4444;}"
                  << ".node-label{font-weight:500;margin-right:12px;}"
                  << ".node-branch{font-weight:600;}"
                  << ".node-counts{display:flex;gap:6px;font-size:12px;color:#666;}"
                  << ".count-imp::before{content:'[';}"
                  << ".count-imp::after{content:'] imp ';}"
                  << ".count-fail::after{content:' fail ';}"
                  << ".count-pass::after{content:' pass ';}"
                  << ".count-abort::after{content:' ab';}"
                  << ".tree-children{border-left:1px solid #ddd;margin-left:20px;padding-left:8px;}"
                  << ".tree-children[style*=\"display: none\"] {display:none !important;}"
                  << ".tree-children[style*=\"display:block\"] {display:block !important;}"
                  << "</style></head><body>" << nav
                  << "<h1>Bot Test Tree</h1>"
                  << "<p>Tests from the last server run. Click ▶ to expand, ▼ to collapse.</p>"
                  << "<script>"
                  << "function toggleNode(btn){var children=btn.parentElement.nextElementSibling;if(children&&children.classList.contains('tree-children')){var isHidden=children.style.display==='none';children.style.display=isHidden?'block':'none';var icon=btn.querySelector('.toggle-icon');icon.textContent=isHidden?'▼':'▶';}}"
                  << "</script>";

        std::vector<TestTreeNode*> treeRoots = BuildTestTree(summary.allRecords);
        testsPage << GenerateTestTreeHtml(treeRoots);
        DeleteTestTree(treeRoots);

        testsPage << "</body></html>";
        WriteTextFile(JoinPath(reportDir, "tests.html"), testsPage.str());

        std::ostringstream landing;
        landing << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Bot Test Status</title>"
                << "<style>body{font-family:Segoe UI,Arial,sans-serif;margin:24px;} ul{line-height:1.8;}"
                << "</style></head><body><h1>Bot Test Status</h1>"
                << "<p>Generated at: " << HtmlEscape(summary.generatedAt) << "</p>"
                << "<p>This report site is regenerated on each server boot.</p>"
                << "<ul>"
                << "<li><a href=\"" << REPORT_SUBDIR << "/overview.html\">Overview</a></li>"
                << "<li><a href=\"" << REPORT_SUBDIR << "/tests.html\">Tests (Tree View)</a></li>"
                << "<li><a href=\"" << REPORT_SUBDIR << "/failures.html\">Failures</a></li>"
                << "<li><a href=\"" << REPORT_SUBDIR << "/history.html\">History</a></li>"
                << "</ul></body></html>";
        WriteTextFile(JoinPath(logsDir, LANDING_PAGE_FILE), landing.str());
    }
}

void LogAnalysis::RunAnalysis()
{
    AnalysePid();
    AnalyseEvents();
    AnalyseQuests();
    AnalyseCounts();
    AnalyseTestResults();
}

void LogAnalysis::AnalyseTestResults()
{
    const std::string logsDir = GetLogsDir();
    const std::string testLogPath = JoinPath(logsDir, TEST_RESULTS_FILE);
    const std::string historyPath = JoinPath(logsDir, HISTORY_FILE);

    TestSummary summary;
    summary.generatedAt = sPlayerbotAIConfig.GetTimestampStr();

    std::ifstream in(testLogPath.c_str(), std::ifstream::in);
    if (!in.fail())
    {
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;

            TestRecord record;
            if (!ParseTestRecord(line, record))
                continue;

            ++summary.total;

            summary.allRecords.push_back(record);

            if (record.result == "PASS")
            {
                ++summary.pass;
            }
            else if (record.result == "FAIL")
            {
                ++summary.fail;
                ++summary.failingTests[record.testName];
                summary.failingRecords[record.testName].push_back(record);
            }
            else if (record.result == "ABORT")
            {
                ++summary.abortCount;
                ++summary.failingTests[record.testName];
                summary.failingRecords[record.testName].push_back(record);
            }
            else if (record.result == "IMPOSSIBLE")
            {
                ++summary.impossible;
                ++summary.failingTests[record.testName];
                summary.failingRecords[record.testName].push_back(record);
            }
            else
            {
                ++summary.impossible;
                ++summary.failingTests[record.testName];
                summary.failingRecords[record.testName].push_back(record);
            }
        }
    }

    std::ostringstream out;
    out << summary.generatedAt << ",TEST"
        << ",{total," << summary.total << "}"
        << ",{pass," << summary.pass << "}"
        << ",{fail," << summary.fail << "}"
        << ",{abort," << summary.abortCount << "}"
        << ",{impossible," << summary.impossible << "}"
        << ",{passRate," << FormatPassRate(summary.pass, summary.total) << "}";

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());

    std::vector<HistoryRow> historyRows = LoadHistoryRows(historyPath);

    HistoryRow currentRow;
    currentRow.timestamp = summary.generatedAt;
    currentRow.total = summary.total;
    currentRow.pass = summary.pass;
    currentRow.fail = summary.fail;
    currentRow.abortCount = summary.abortCount;
    currentRow.impossible = summary.impossible;
    currentRow.passRate = FormatPassRate(summary.pass, summary.total);
    currentRow.topFailingTest = TopFailingTest(summary.failingTests);

    AppendHistoryRow(historyPath, currentRow);
    historyRows.push_back(currentRow);

    GenerateReportSite(logsDir, summary, historyRows);

    sLog.outString("Generated bot test report: %s", JoinPath(logsDir, LANDING_PAGE_FILE).c_str());
}

void LogAnalysis::AnalysePid()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir+"activity_pid.csv", std::ifstream::in);

    std::vector<uint32> activeBots, totalBots, avgDiff;
    
    uint32 runTime, maxBots = 0, start=0;

    if (in.fail())
        return;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        if (tokens.size() < 7)
            continue;

        if (tokens.size() > 50)//Multiple servers running
            continue;
        
        if (tokens[0] == "Timestamp") //Header line
            continue;

        activeBots.push_back(stoi(tokens[6]));
        totalBots.push_back(stoi(tokens[7]));
        avgDiff.push_back(stoi(tokens[2]));
        if (tokens[0][0] == '.')
            tokens[0] = "0" + tokens[0];

        if (!Qualified::isValidNumberString(tokens[0]))
            continue;

        runTime = stoi(tokens[0]);

        if ((uint32)stoi(tokens[7]) > maxBots)
        {
            maxBots = (uint32)stoi(tokens[7]);
            start = avgDiff.size();
        }
    }
    while (in.good());

    if (activeBots.empty() || start == activeBots.size())
        return;

    uint32 aDiff = 0, aBots =0;
    for (uint32 i = start; i < activeBots.size(); i++)
    {
        aDiff += avgDiff[i];
        aBots += activeBots[i];
    }

    aDiff /= (activeBots.size()- start);
    aBots /= (activeBots.size()- start);

    using namespace std::chrono;
    std::chrono::milliseconds ms(runTime);
    auto secs = duration_cast<seconds>(ms);
    ms -= duration_cast<milliseconds>(secs);
    auto mins = duration_cast<minutes>(secs);
    secs -= duration_cast<seconds>(mins);
    auto hour = duration_cast<hours>(mins);
    mins -= duration_cast<minutes>(hour);

    std::stringstream ss;
    ss << hour.count() << " Hours : " << mins.count() << " Minutes : " << secs.count() << " Seconds ";
    
    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr()  << "," << "PID" << "," << ss.str().c_str() << "," << aDiff << "," << aBots << "," << maxBots;

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());

    sLog.outString("========= LAST SERVER RUNTIME: %s", ss.str().c_str());
    sLog.outString("========= AVG DIFF [%d] & AVG ACTIVE BOTS [%d/%d]", aDiff, aBots, maxBots);
}

void LogAnalysis::AnalyseEvents()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir + "bot_events.csv", std::ifstream::in);

    if (in.fail())
        return;

    std::map<std::string, uint32> eventCount, eventMin;

    eventMin["AcceptInvitationAction"] = 1;
    eventMin["AcceptQuestAction"] = 1;
    eventMin["AutoReleaseSpiritAction"] = 1;
    eventMin["AttackAnythingAction"] = 1;
    eventMin["AutoSetTalentsAction"] = 1;
    eventMin["BGJoinAction"] = 1;
    eventMin["CheckMountStateAction"] = 1;    
    eventMin["EquipAction"] = 1;
    eventMin["LeaveGroupAction"] = 1;
    eventMin["QueryItemUsageAction"] = 1;
    eventMin["QuestUpdateAddKillAction"] = 1;
    eventMin["ReviveFromCorpseAction"] = 1;
    eventMin["StoreLootAction"] = 1;
    eventMin["SellAction"] = 1;
    eventMin["BuyAction"] = 1;
    eventMin["AhAction"] = 1;
    eventMin["AhBidAction"] = 1;
    eventMin["MailAction"] = 1;
    eventMin["TalkToQuestGiverAction"] = 1;
    eventMin["TrainerAction"] = 1;
    eventMin["XpGainAction"] = 1;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        eventCount[tokens[2]]++;
    } while (in.good());

    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr() << "," << "EVENT";

    for (auto event : eventMin)
    {
        out << ",{" << event.first << "," << eventCount[event.first] << "}";

        if (eventCount[event.first] < event.second)
            sLog.outError("Event %s was only seen %d times", event.first.c_str(), eventCount[event.first]);
    }

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());
}

void LogAnalysis::AnalyseQuests()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir + "bot_events.csv", std::ifstream::in);


    if (in.fail())
        return;

    std::map<std::string, uint32> questId, questStartCount, questObjective, questCompletedCount, questEndCount;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        if (tokens.size() == 10) //Some quest names have a "," so add an extra element.
        {
            tokens[7] = tokens[7] + tokens[8];
            tokens[8] = tokens[9];
        }
        else if (tokens.size() == 11)
        {
            tokens[7] = tokens[7] + tokens[8] + tokens[9];
            tokens[8] = tokens[10];
        }


        for(auto& token : tokens)
            token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());

        if (tokens[2] == "AcceptQuestAction")
        {
            questId[tokens[7]] = stoi(tokens[8]);
            questStartCount[tokens[7]]++;
        }
        else if (tokens[2] == "QueryItemUsageAction" || tokens[3] == "QuestUpdateAddKillAction")
        {
            questId[tokens[7]] = stoi(tokens[8]);
            questObjective[tokens[7]]++;
            if (stof(tokens[8]) == 1)
                questCompletedCount[tokens[7]]++;
        }
        else if (tokens[2] == "TalkToQuestGiverAction")
        {
            questId[tokens[7]] = stoi(tokens[8]);
            questEndCount[tokens[7]]++;
        }
    } while (in.good());

    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr() << "," << "QUEST";

    for (auto quest : questId)
    {
        out << ",{" << quest.second << "," << quest.first << "," << questStartCount[quest.first] << "," << questObjective[quest.first] << "," << questObjective[quest.first] << ","<< questEndCount[quest.first] <<"}";

        if (!questEndCount[quest.first])
        {
            if (questCompletedCount[quest.first] > 1)
                sLog.outError("Quest %s (%d) has %d objectives completed but was never turned in.", quest.first.c_str(), quest.second, questCompletedCount[quest.first]);
            else if (questObjective[quest.first] > 1)
                sLog.outError("Quest %s (%d) has %d objectives partially completed but was never turned in.", quest.first.c_str(), quest.second, questObjective[quest.first]);
            else if (questStartCount[quest.first] > 10)
                sLog.outError("Quest %s (%d) started %d times but never turned in.", quest.first.c_str(), quest.second, questStartCount[quest.first]);
        }
    }

    sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());
}

void LogAnalysis::AnalyseCounts()
{
    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }
    std::ifstream in(m_logsDir + "bot_events.csv", std::ifstream::in);

    if (in.fail())
        return;

    uint32 maxShow = 10;

    std::map<std::string, std::string> countType;
    countType["TalkToQuestGiverAction"] = "Quest completed";
    countType["EquipAction"] = "Item equiped";
    countType["StoreLootAction"] = "Item looted";
    countType["BuyAction"] = "Item bought";
    countType["SellAction"] = "Item sold";
    countType["AhAction"] = "Item sold on AH";
    countType["AhBidAction"] = "Item bid on AH";
    countType["MailAction"] = "Item bought on AH";
    countType["TrainerAction"] = "Skill learned";   
    countType["AttackAnythingAction"] = "Mob attacked";
    countType["XpGainAction"] = "Mob killed";

    std::map<std::string, std::map<std::string, uint32>> counts;

    do
    {
        std::string line;
        std::getline(in, line);

        if (!line.length())
            continue;

        Tokens tokens = StrSplit(line, ",");

        if (tokens.size() == 10) //Some quest names have a "," so add an extra element. 
        {
            tokens[7] = tokens[7] + tokens[8];
            tokens[8] = tokens[9];
            tokens.pop_back();
        }

        for (auto& token : tokens)
            token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());

        if (countType[tokens[2]].empty())
            continue;

        if (tokens[7].empty())
            continue;

        counts[tokens[2]][tokens[7]]++;        
    } while (in.good());
  
    for (auto& count : counts)
    {

        std::vector<std::pair<std::string, uint32>> list;
        std::ostringstream out;

        out << sPlayerbotAIConfig.GetTimestampStr() << "," << "COUNT-" << count.first;

        for (auto type : count.second)
            list.push_back(make_pair(type.first, type.second));

        std::sort(list.begin(), list.end(), [](std::pair<std::string, uint32> i, std::pair<std::string, uint32> j) { return i.second > j.second; });

        for(auto& l:list)
            out << ",{" << l.first << "," << l.second << "}";

        sPlayerbotAIConfig.log("log_analysis.csv", out.str().c_str());

        sLog.outString("[Top %d %s]", maxShow, countType[count.first].c_str());
        for (uint32 i = 0; i < std::min(maxShow, (uint32)list.size()); i++)
            sLog.outString("%s (%d)", list[i].first.c_str(), list[i].second);
    }
}