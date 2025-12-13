#include "DJLibraryService.h"

#include <filesystem>
#include <iostream>
#include <memory>

#include "MP3Track.h"
#include "PointerWrapper.h"
#include "SessionFileParser.h"
#include "WAVTrack.h"

DJLibraryService::DJLibraryService()
    : playlist(), library() {}  // YA empty vector

DJLibraryService::DJLibraryService(const Playlist& playlist)  // YA copt ctor
    : playlist(playlist), library() {}                        // YA empty vector

DJLibraryService::~DJLibraryService() {
  // YA-this is the owner so we must delete
  for (AudioTrack* track : library) {
    delete track;  // YA relese the memory
  }
  library.clear();  // YA clean the vector
}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */
void DJLibraryService::buildLibrary(
    const std::vector<SessionConfig::TrackInfo>& library_tracks) {
  std::cout << "[INFO] Building track library from config...\n";

  // YA clean the library for making sure there is no duplication
  for (AudioTrack* t : library) {
    delete t;
  }
  library.clear();

  // YA check the trackInfo
  for (const auto& info : library_tracks) {
    // YA copy the artists to a new vector
    std::vector<std::string> artists = info.artists;

    AudioTrack* track = nullptr;  // YA pointer to the track we will creat

    if (info.type == "MP3") {
      track = new MP3Track(info.title,
                           artists,  // YA vector string of artists
                           info.duration_seconds, info.bpm,
                           info.extra_param1  // bitrate
      );
    } else if (info.type == "WAV") {
      track = new WAVTrack(info.title,
                           artists,  // YA vector string of artists
                           info.duration_seconds, info.bpm,
                           info.extra_param1,  // sample_rate
                           info.extra_param2   // bit_depth
      );

    } else {
      std::cerr << "[ERROR] Unknown track type: " << info.type << "\n";
      continue;  // YA didnt add nothing
    }

    // YA library is the ownership
    library.push_back(track);
  }

  std::cout << "[INFO] Track library built: " << library.size()
            << " tracks loaded\n";
}

/**
 * @brief Display the current state of the DJ library playlist
 *
 */
void DJLibraryService::displayLibrary() const {
  std::cout << "=== DJ Library Playlist: " << playlist.get_name()
            << " ===" << std::endl;

  if (playlist.is_empty()) {
    std::cout << "[INFO] Playlist is empty.\n";
    return;
  }

  // Let Playlist handle printing all track info
  playlist.display();

  std::cout << "Total duration: " << playlist.get_total_duration() << " seconds"
            << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 *
 * @return Playlist&
 */
Playlist& DJLibraryService::getPlaylist() {  // YA we dont need to change
  // Your implementation here
  return playlist;
}

/**
 * TODO: Implement findTrack method
 *
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
  return playlist.find_track(
      track_title);  // YA using the playlist method-find track
}

void DJLibraryService::loadPlaylistFromIndices(
    const std::string& playlist_name, const std::vector<int>& track_indices) {
  std::cout << "[INFO] Loading playlist: " << playlist_name << "\n";

  // YA new playlist
  playlist = Playlist(playlist_name);

  int added_count = 0;

  for (auto it = track_indices.begin(); it != track_indices.end();
       ++it) {  // YA loop by idx
    int idx = *it;
    int zero_based = idx - 1;

    if (zero_based < 0 || zero_based >= (int)library.size()) {
      std::cerr << "[WARNING] Invalid track index: " << idx << "\n";
      continue;
    }

    AudioTrack* track = library[zero_based];
    if (!track) {
      std::cerr << "[ERROR] Null track pointer at index: " << idx << "\n";
      continue;
    }
    // YA - load & analize
    track->load();
    track->analyze_beatgrid();

    playlist.add_track(track);
    ++added_count;
  }
  std::cout << "[INFO] Playlist loaded: " << playlist_name << " ("
            << added_count << " tracks)\n";
}

/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
  std::vector<std::string> titles;  // YA vector for the titles

  std::vector<AudioTrack*> tracks =
      playlist.getTracks();  // YA take all the *AudioTrack from the playlist

  titles.reserve(tracks.size());  // YA set the place before

  for (AudioTrack* t : tracks) {         // YA for every track
    if (t) {                             // YA making sure it isnt null
      titles.push_back(t->get_title());  // YA pushing the title to the vector
    }
  }

  return titles;  // YA return the titles vector
}
