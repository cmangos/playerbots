# Playerbots
Bot AI Core from ike3 for cmangos classic, tbc and wotlk

💡  If you're new to building CMaNGOS, check the official Windows guide
https://github.com/cmangos/issues/wiki/Detailed-installation-guide-for-Microsoft-Windows

Important: to enable the playerbots you need to check it in cmake ( BUILD_PLAYERBOTS ✅ )

💡  After successful build get aiplayerbot.conf file from "src/modules/Bots/playerbot/aiplayerbot%expansion.conf.dist" (based on expansion you use) and put it to the same folder where mangosd.conf and realmd.conf are, and remove ".dist" from its name

💡  DB modifications:

  a)  Go to "src/modules/Bots/sql"
  
  b)  Apply .sql files from "characters" folder to characters database
  
  c)  Apply .sql files from "world" folder to world database
  
  **IMPORTANT**: playebot folder has several .sql files that have `vanilla`, `tbc` or `wotlk` in their names. You should **apply only ONE** of them, corresponding to expansion you use.

After you complete all steps above you can check bots config and start your server. It'll take some time for the first time, as gear/characters for bots will be generated at first launch. Have fun! 🥳
