export const ButtonEnum: {
  Play: 0
  Pause: 1
  Stop: 2
  Record: 3
  FastForward: 4
  Rewind: 5
  Next: 6
  Previous: 7
  ChannelUp: 8
  ChannelDown: 9
  Shuffle: 10
  Repeat: 11
}

export const PlaybackStateEnum: {
  Closed: 0
  Changing: 1
  Stopped: 2
  Playing: 3
  Paused: 4
}

export type PlayerDetails = {
  title?: string
  artistName?: string
  albumName?: string
  albumArtist?: string
  thumbnail?: string
  genres?: string[]
  // In seconds
  duration?: number
}
export type PlayerButtons = {
  play?: boolean
  pause?: boolean
  next?: boolean
  prev?: boolean
  seek?: boolean
  shuffle?: boolean
  loop?: "None" | "Track" | "Playlist"
}

export class MediaController {
  createPlayer(name?: string): void
  updatePlayerDetails(obj: PlayerDetails): void
  setButtonStatus(obj: PlayerButtons): void
  setButtonPressCallback(
    callback: (arg: typeof ButtonEnum, arg1?: boolean | number | string) => void
  ): void
  setPlaybackStatus(state: typeof PlaybackStateEnum): void
  setCurrentDuration(duration: number): void
  getPlayer(): string
}
