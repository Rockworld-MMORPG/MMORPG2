fetchcontent_declare(
  discord-gamesdk
  URL https://dl-game-sdk.discordapp.net/2.5.6/discord_game_sdk.zip
  URL_HASH MD5=f86f15957cc9fbf04e3db10462027d58)

fetchcontent_makeavailable(discord-gamesdk)

set(discord-gamesdk_SOURCE_DIR
    ${discord-gamesdk_SOURCE_DIR}
    CACHE INTERNAL "")
