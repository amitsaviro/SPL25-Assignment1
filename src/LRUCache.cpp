#include "LRUCache.h"

#include <iostream>

LRUCache::LRUCache(size_t capacity)
    : slots(capacity), max_size(capacity), access_counter(0) {}

bool LRUCache::contains(const std::string& track_id) const {
  return findSlot(track_id) != max_size;
}

AudioTrack* LRUCache::get(const std::string& track_id) {
  size_t idx = findSlot(track_id);
  if (idx == max_size) return nullptr;
  return slots[idx].access(++access_counter);
}

/**
 * TODO: Implement the put() method for LRUCache
 */
bool LRUCache::put(PointerWrapper<AudioTrack> track) {
  if (!track.get()) return false;  // YA - if ptrWrapper empty do nothing

  size_t existing = findSlot(
      track->get_title());  // YA - returns the index of the track or max_size
  if (existing != max_size) {  // if we got an index than track is already there
    slots[existing].access(++access_counter);  // updates Recently Used
    return false;
  }

  size_t empty = findEmptySlot();  // if not in the slot already
  bool eviction = false;

  if (empty ==
      max_size) {  // YA - if all the slots are taken needs to free the RLU
    empty = findLRUSlot();
    eviction = evictLRU();  // removes the RLU and returns true
  }
  // YA - empty = empty slot or the one we removed the RLU from
  slots[empty].store(std::move(track),
                     ++access_counter);  // YA - stores the track and gives the
                                         // ownership to the slot (move track),
                                         // increases the counter

  return eviction;
}

bool LRUCache::evictLRU() {
  size_t lru = findLRUSlot();
  if (lru == max_size || !slots[lru].isOccupied()) return false;
  slots[lru].clear();
  return true;
}

size_t LRUCache::size() const {
  size_t count = 0;
  for (const auto& slot : slots)
    if (slot.isOccupied()) ++count;
  return count;
}

void LRUCache::clear() {
  for (auto& slot : slots) {
    slot.clear();
  }
}

void LRUCache::displayStatus() const {
  std::cout << "[LRUCache] Status: " << size() << "/" << max_size
            << " slots used\n";
  for (size_t i = 0; i < max_size; ++i) {
    if (slots[i].isOccupied()) {
      std::cout << "  Slot " << i << ": " << slots[i].getTrack()->get_title()
                << " (last access: " << slots[i].getLastAccessTime() << ")\n";
    } else {
      std::cout << "  Slot " << i << ": [EMPTY]\n";
    }
  }
}

size_t LRUCache::findSlot(const std::string& track_id) const {
  for (size_t i = 0; i < max_size; ++i) {
    if (slots[i].isOccupied() && slots[i].getTrack()->get_title() == track_id)
      return i;
  }
  return max_size;
}

/**
 * TODO: Implement the findLRUSlot() method for LRUCache
 */
size_t LRUCache::findLRUSlot() const {
  uint64_t min_time = UINT64_MAX;  // YA - every time we will update the minimum
                                   // (which is the LRU)
  size_t lru_index = max_size;

  for (size_t i = 0; i < max_size; ++i) {
    if (slots[i].isOccupied() && slots[i].getLastAccessTime() < min_time) {
      min_time =
          slots[i].getLastAccessTime();  // YA - if smaller than he is the LRU
      lru_index = i;                     // YA - returns the LRU index
    }
  }

  return lru_index;
}

size_t LRUCache::findEmptySlot() const {
  for (size_t i = 0; i < max_size; ++i) {
    if (!slots[i].isOccupied()) return i;
  }
  return max_size;
}

void LRUCache::set_capacity(size_t capacity) {
  if (max_size == capacity) return;
  // udpate max size
  max_size = capacity;
  // update the slots vector
  slots.resize(capacity);
}