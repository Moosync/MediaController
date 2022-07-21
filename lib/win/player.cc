#include "player.h"
#include "utils.h"
#include <iostream>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Media.Control.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Media;
using namespace Windows::Media::Playback;
using namespace Windows::Media::Control;

Napi::Object Player::Init(Napi::Env env, Napi::Object exports) {
	Napi::Function func = DefineClass(env, "Player", {
																											 InstanceMethod("getPlayer", &Player::getPlayer),
																											 InstanceMethod("createPlayer", &Player::createPlayer),
																											 InstanceMethod("updatePlayerDetails", &Player::updatePlayerDetails),
																											 InstanceMethod("setButtonStatus", &Player::setButtonStatus),
																											 InstanceMethod("setButtonPressCallback", &Player::setButtonPressCallback),
																											 InstanceMethod("setPlaybackStatus", &Player::setPlaybackStatus)
																									 });

	Napi::FunctionReference* constructor = new Napi::FunctionReference();
	*constructor = Napi::Persistent(func);
	env.SetInstanceData(constructor);

	exports.Set("MediaController", func);
	return exports;
}


Napi::Value Player::setPlaybackStatus(const Napi::CallbackInfo &info) {
	auto env = info.Env();

	if (info.Length() != 1 || !info[0].IsNumber()) {
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	auto val = info[0].ToNumber().Int32Value();
	if (val >= 0 && val <= 4 && this->mediaPlayer.has_value()) {
		const auto systemMediaTransportControls = this->mediaPlayer->SystemMediaTransportControls();
		systemMediaTransportControls.PlaybackStatus(static_cast<MediaPlaybackStatus>(val));
	}

	return env.Undefined();
}

Napi::Value Player::setButtonStatus(const Napi::CallbackInfo &info) {
	auto env = info.Env();

	if (info.Length() != 1 || !info[0].IsObject()) {
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	auto obj = info[0].As<Napi::Object>();

	if (this->mediaPlayer.has_value()) {
		std::cout << "Val " << getBool(obj, "next");

		const auto systemMediaTransportControls = this->mediaPlayer->SystemMediaTransportControls();
		systemMediaTransportControls.IsEnabled(true);
		systemMediaTransportControls.IsPlayEnabled(getBool(obj, "play"));
		systemMediaTransportControls.IsPauseEnabled(getBool(obj, "pause"));
		systemMediaTransportControls.IsNextEnabled(getBool(obj, "next"));
		systemMediaTransportControls.IsPreviousEnabled(getBool(obj, "prev"));
	}

	return env.Undefined();
}


Napi::Value Player::setButtonPressCallback(const Napi::CallbackInfo &info) {
	auto env = info.Env();

	if (info.Length() != 1 || !info[0].IsFunction()) {
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	// https://github.com/NyaomiDEV/WinPlayer-Node/blob/master/src/wrapper.cpp#L31
	// Their tears didn't goto waste
	auto tsfn = Napi::ThreadSafeFunction::New(
		info.Env(),
		info[0].As<Napi::Function>(),
		"Callback",
		0,
		1
	);

	this->mediaPlayer->SystemMediaTransportControls().ButtonPressed(
			[tsfn](SystemMediaTransportControls sender, SystemMediaTransportControlsButtonPressedEventArgs args)
			{
				auto val = static_cast<int32_t>(args.Button());
				auto callback = [val](Napi::Env env, Napi::Function jsCallback)
				{ jsCallback.Call({Napi::Number::New(env, val)}); };
				tsfn.NonBlockingCall(callback);
			});

	return env.Undefined();
}

Player::Player(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Player>(info) {}

Napi::Value Player::createPlayer(const Napi::CallbackInfo &info){
	auto env = info.Env();

	auto mediaPlayer = MediaPlayer();
	this->mediaPlayer = mediaPlayer;
	return env.Undefined();
}

Napi::Value Player::updatePlayerDetails(const Napi::CallbackInfo &info) {
	auto env = info.Env();

	if (info.Length() != 1) {
		Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
		return env.Null();
	}

	const auto mediaInfo = toMediaInfo(info[0].As<Napi::Object>());

	const auto commandManager = this->mediaPlayer->CommandManager();
	commandManager.IsEnabled(false);

	const auto systemMediaTransportControls = this->mediaPlayer->SystemMediaTransportControls();
	systemMediaTransportControls.IsEnabled(true);
	systemMediaTransportControls.IsPlayEnabled(true);
	systemMediaTransportControls.IsPauseEnabled(true);
	systemMediaTransportControls.PlaybackStatus(MediaPlaybackStatus::Playing);

	auto updater = systemMediaTransportControls.DisplayUpdater();
	updater.ClearAll();
	updater.Type(MediaPlaybackType::Music);
	auto properties = updater.MusicProperties();

	if (mediaInfo.Title.has_value()) {
		properties.Title(to_hstring(mediaInfo.Title.value().Utf8Value()));
	}

	if (mediaInfo.ArtistName.has_value()) {
		properties.Artist(to_hstring(mediaInfo.ArtistName.value().Utf8Value()));
	}


	if (mediaInfo.AlbumName.has_value()) {
		properties.AlbumTitle(to_hstring(mediaInfo.AlbumName.value().Utf8Value()));
	}


	if (mediaInfo.AlbumArtist.has_value()) {
		properties.AlbumArtist(to_hstring(mediaInfo.AlbumArtist.value().Utf8Value()));
	}

	auto genres = properties.Genres();
	genres.Clear();

	if (mediaInfo.Genres.has_value()) {
		for (auto genre: mediaInfo.Genres.value()) {
			genres.Append(to_hstring(static_cast<Napi::Value>(genre.second).ToString().Utf8Value()));
		}
	}

	if (mediaInfo.Thumbnail.has_value()) {
		if (isNetworkUri(mediaInfo.Thumbnail.value().Utf8Value())) {
			updater.Thumbnail(RandomAccessStreamReference::CreateFromUri(Uri(to_hstring(mediaInfo.Thumbnail.value().Utf8Value()))));
		} else {
			auto asyncOp = StorageFile::GetFileFromPathAsync(to_hstring(mediaInfo.Thumbnail.value().Utf8Value()));
			updater.Thumbnail(RandomAccessStreamReference::CreateFromFile(asyncOp.get()));
		}
	}

	updater.Update();

	return env.Undefined();
}

Napi::Value Player::getPlayer(const Napi::CallbackInfo &info){
	auto env = info.Env();

	auto sessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();

	auto current = sessionManager.GetCurrentSession();
	if (current != NULL) {
		return Napi::String::New(env, to_string(current.SourceAppUserModelId()));
	}

	return env.Undefined();
}