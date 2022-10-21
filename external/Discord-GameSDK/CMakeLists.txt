fetchcontent_declare(
  discord-gamesdk
  URL https://dl-game-sdk.discordapp.net/3.2.1/discord_game_sdk.zip
  URL_HASH MD5=73e5e1b3f8413a2c7184ef17476822f2)

fetchcontent_makeavailable(discord-gamesdk)

set(discord-gamesdk_SOURCE_DIR
    ${discord-gamesdk_SOURCE_DIR}
    CACHE INTERNAL "")
