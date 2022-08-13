const { ButtonEnum, PlaybackStateEnum } = require('./constants')
module.exports = { ButtonEnum, PlaybackStateEnum }

if (process.platform === "win32") {
  module.exports['MediaController'] = require('./win').MediaController
} else if (process.platform === "linux") {
  module.exports['MediaController'] = require('./linux').MediaController
} else {
  module.exports['MediaController'] = require('./dummy').MediaController
}


