const { ButtonEnum, PlaybackStateEnum } = require('./constants')
module.exports = { ButtonEnum, PlaybackStateEnum }

if (process.platform === "win32") {
  module.exports['MediaController'] = require('./win').MediaController
}

if (process.platform === "linux") {
  module.exports['MediaController'] = require('./linux').MediaController
}
