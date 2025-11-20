#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "AudioTrack.h"
#include <string>
#include <vector>

/**
 * ⚠️  WARNING: THIS CLASS HAS INTENTIONAL MEMORY LEAKS! ⚠️
 * 
 * This is Phase 1 of the assignment - students must identify and fix all memory leaks.
 * @todo Implement proper memory management to prevent leaks.
 * @note In phase 4, the library service should provide canonical ownership semantics
 * for tracks referenced by playlists. Fixes in earlier phases should ensure
 * clear ownership and safe iteration without leaks.
 */

struct PlaylistNode {
    AudioTrack* track; 
    PlaylistNode* next;

    PlaylistNode(AudioTrack* t) : track(t), next(nullptr) {}
    ~PlaylistNode() = default;
};

class Playlist {    
private:
    PlaylistNode* head;
    std::string playlist_name;
    int track_count;

     // YA: Helper function - delete all PlaylistNode objects (but NOT the AudioTrack objects)
    void clear();

    // YA Helper function - copy linked list structure from another playlist
    // Creates new nodes that point to the same AudioTrack* as in 'other'
    void copy_from(const Playlist& other);

public:
    /**
     * Constructor
     */
    Playlist(const std::string& name="");

    /**
     * Destructor
     */
     // YA - the rule of 3
    ~Playlist();                                // destructor
    Playlist(const Playlist& other);            // copy constructor
    Playlist& operator=(const Playlist& other); // copy assignment

    /**
     * Add a track to the playlist
     * @param track Pointer to AudioTrack to add
     */
    void add_track(AudioTrack* track);

    /**
     * Remove a track by title
     * @param title Title of the track to remove
     */
    void remove_track(const std::string& title);

    /**
     * Display all tracks in the playlist
     */
    void display() const;

    /**
     * Get playlist statistics
     * @return Number of tracks in the playlist
     */
    int get_track_count() const { return track_count; }
    const std::string& get_name() const { return playlist_name; }

    /**
     * @param title Title of the track to find
     * @brief Find a track by title
     * @return Pointer to the found track, or nullptr if not found
     */
    AudioTrack* find_track(const std::string& title) const;

    /**
     * Check if playlist is empty
     */
    bool is_empty() const { return head == nullptr; }

    /**
     * Calculate total duration of all tracks
     */
    int get_total_duration() const;

    /**
     * Get all tracks as a vector
     */
    std::vector<AudioTrack*> getTracks() const;

    };
#endif // PLAYLIST_H
