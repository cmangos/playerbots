# Playerbots
Bot AI Core from ike3 for cmangos classic, tbc and wotlk

💡  If you're new to building CMaNGOS, check the official guide
https://github.com/cmangos/issues/wiki/Installation-Instructions

Important: to enable the playerbots you need to check it in cmake ( `BUILD_PLAYERBOTS` ✅ )

💡  After successful build get aiplayerbot.conf file from "src/modules/Bots/playerbot/aiplayerbot%expansion.conf.dist" (based on expansion you use) and put it to the same folder where mangosd.conf and realmd.conf are, and remove ".dist" from its name

💡  Apply DB modifications:
- Using the `InstallFullDB.sh` script:
  1. Execute the script once to generate the `InstallFullDB.config` file, after that close the script
  2. Edit `InstallFullDB.config` and add `PLAYERBOTS_DB="YES"` at the end and save it
  3. Depending on if you have a previous installation or want to do a fresh installation:
     - For a fresh install pick the option `4) Full installation`
     - To install only the playerbots DB pick the option `5) Advanced DB management` and then `8) Create and fill playerbots db`

- Manually:
  1.  Go to "src/modules/Bots/sql"
  2.  Apply .sql files from "characters" folder to characters database
  3.  Apply .sql files from "world" folder to world database
  
  **IMPORTANT**: There are several .sql files that are in a `vanilla`, `tbc` or `wotlk` folder. You should only use the files in the folder for the core expansion you are currently using.

After you complete all steps above you can check bots config and start your server. It'll take some time for the first time, as gear/characters for bots will be generated at first launch. Have fun! 🥳
