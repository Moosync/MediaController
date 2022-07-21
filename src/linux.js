const { ButtonEnum, PlaybackStateEnum } = require('./constants')

const Player = require("mpris-service")

class MediaController {
  player = undefined
  callbackMap = {}

  createPlayer () {
    this.player = Player({
      name: "nodejs",
      identity: "Node.js media player",
      supportedUriSchemes: ["file", "https", "http"],
      supportedMimeTypes: ["audio/mpeg", "application/ogg"],
      supportedInterfaces: ["player"],
    })
  }

  updatePlayerDetails (obj) {
    this.player.metadata = {
      "mpris:artUrl": obj.thumbnail,
      "xesam:title": obj.title,
      "xesam:album": obj.albumName,
      "xesam:artist": obj.artistName.split(","),
    }
  }

  setButtonStatus (obj) {
    const { play, pause, next, prev } = obj
    this.player.canPlay = !!play
    this.player.canPause = !!pause
    this.player.canGoNext = !!next
    this.player.canGoPrevious = !!prev
  }

  setButtonPressCallback (callback) {
    for (const c in this.callbackMap) {
      this.player.removeListener(c, this.callbackMap[c])
    }

    this.callbackMap["next"] = this.player.on("next", () =>
      callback(ButtonEnum.Next)
    )
    this.callbackMap["previous"] = this.player.on("previous", () =>
      callback(ButtonEnum.Previous)
    )
    this.callbackMap["pause"] = this.player.on("pause", () =>
      callback(ButtonEnum.Pause)
    )
    this.callbackMap["play"] = this.player.on("play", () =>
      callback(ButtonEnum.Play)
    )
    this.callbackMap["stop"] = this.player.on("stop", () =>
      callback(ButtonEnum.Stop)
    )
    this.callbackMap["shuffle"] = this.player.on("shuffle", (arg) =>
      callback(ButtonEnum.Shuffle, arg)
    )
    this.callbackMap["loopStatus"] = this.player.on("loopStatus", (arg) =>
      callback(ButtonEnum.Repeat, arg)
    )
  }

  setPlaybackStatus (state) {
    if (state === PlaybackStateEnum.Playing) {
      this.player.playbackStatus = Player.PLAYBACK_STATUS_PLAYING
    } else if (state === PlaybackStateEnum.Paused) {
      this.player.playbackStatus = Player.PLAYBACK_STATUS_PAUSED
    } else if (state === PlaybackStateEnum.Stopped) {
      this.player.playbackStatus = Player.PLAYBACK_STATUS_STOPPED
    } else if (state === PlaybackStateEnum.Changing) {
      this.player.playbackStatus = Player.PLAYBACK_STATUS_PAUSED
    } else if (state === PlaybackStateEnum.Closed) {
      this.player.playbackStatus = Player.PLAYBACK_STATUS_STOPPED
    }
  }

  setShuffleRepeat (shuffle = false, repeat = 'None') {
    this.player.shuffle = !!shuffle
    this.player.loopStatus = repeat
  }

  getPlayer () {
    return this.player.name
  }
}

module.exports = { MediaController }