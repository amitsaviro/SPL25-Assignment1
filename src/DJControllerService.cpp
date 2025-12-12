#include "DJControllerService.h"

#include <iostream>
#include <memory>

#include "MP3Track.h"
#include "WAVTrack.h"

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
  // YA - Hit case
  if (cache.contains(track.get_title())) {
    std::cout << "[Cache HIT] " << track.get_title()
              << " found in cache. Refreshing MRU state.\n";
    cache.get(track.get_title());  // updates RU
    return 1;                      // HIT
  }
  std::cout << "[Cache MISS] Cloning track into cache: "
            << track.get_title() << "\n";

  // polymorphic clone
  PointerWrapper<AudioTrack> clone(track.clone());
  if (!clone.get()) {  // YA - if clone is nullptr - Error msg
    std::cerr << "[ERROR] Track: \"" << track.get_title()
              << "\" failed to clone\n";
    return 0;
  }

  // YA - simulates loading and beatgrid - what we did in phase 2!
  clone->load();
  clone->analyze_beatgrid();

  // YA - wrapping clone and inserting to cache using put which we wrote before
  // (removes Least Recently Used)
  bool eviction = cache.put(std::move(clone));
  std::cout << "[Cache INSERT] Added '" << track.get_title()
            << "' to cache.\n";

  if (eviction) {
    return -1;  // YA - Miss with eviction
  } else {
    return 0;  // YA - Miss without eviction
  }
}

void DJControllerService::set_cache_size(size_t new_size) {
  cache.set_capacity(new_size);
}
// implemented
void DJControllerService::displayCacheStatus() const {
  std::cout << "\n=== Cache Status ===\n";
  cache.displayStatus();
  std::cout << "====================\n";
}

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(
    const std::string& track_title) {
  // YA - look up track in cache
  if (!cache.contains(track_title)) {
    return nullptr;  // if not in cache
  }
  // YA - returns raw pointer to track - doesn't change ownership 
  return cache.get(track_title);
}
