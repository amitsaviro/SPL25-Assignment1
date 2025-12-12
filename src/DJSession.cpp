
#include "DJSession.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>

// ========== CONSTRUCTORS & RULE OF 5 ==========


DJSession::DJSession(const std::string& name, bool play_all)//YA ctor
    : session_name(name),
      library_service(),
      controller_service(8), //YA set default for initialize
      mixing_service(),
      config_manager(),
      session_config(),
      track_titles(),
      play_all(play_all),
      stats()
{
    std::cout << "DJ Session System initialized: " << session_name << std::endl;
}



DJSession::~DJSession() {
    std::cout << "Shutting down DJ Session System: " << session_name << std::endl;
}

// ========== CORE FUNCTIONALITY ==========
bool DJSession::load_playlist(const std::string& playlist_name)  {
    std::cout << "[System] Loading playlist: " << playlist_name << "\n";
    
    // Find the playlist in the session config
    auto it = session_config.playlists.find(playlist_name);
    if (it == session_config.playlists.end()) {
        std::cerr << "[ERROR] Playlist '" << playlist_name << "' not found in configuration.\n";
        return false;
    }
    
    // Load playlist from track indices
    library_service.loadPlaylistFromIndices(playlist_name, it->second);
    
    if (library_service.getPlaylist().is_empty()) {
        return false;
    }
    
    track_titles = library_service.getTrackTitles();
    return true;
}

/**
 * TODO: Implement load_track_to_controller method
 * 
 * REQUIREMENTS:
 * 1. Track Retrieval
 *    - Find track in library using track name
 *    - Handle case when track is not found
 *    - Update error stats if track not found
 * 
 * 2. Controller Loading
 *    - Delegate loading to controller_service
 *    - Pass track by reference to controller
 * 
 * 3. Return Values
 *    1: Cache HIT
 *    0: Cache MISS (or error)
 *   -1: Cache MISS with eviction
 * 
 * @param track_name: Name of track to load
 * @return: Cache operation result code

 */
int DJSession::load_track_to_controller(const std::string& track_name) {
    // YA searching the track in the library
    AudioTrack* track = library_service.findTrack(track_name);

    if (!track) {//YA if we didnt find the track
        std::cerr << "[ERROR] Track: \"" << track_name
                  << "\" not found in library\n";
        stats.errors++; //YA error counter ++
        return 0; // 0 = YA MISS with error
    }

    //YA information cout about loading
    std::cout << "[System] Loading track \"" << track_name 
              << "\" to controller...\n";

    //YA send the track to the cache by reference and taking the result
    int result = controller_service.loadTrackToCache(*track);

    //YA checking the results and updating the stats
    if (result == 1) { //YA cache HIT
        stats.cache_hits++;
    } else if (result == 0) {//YA MISS without eviction
        stats.cache_misses++;
    } else if (result == -1) {//YA MISS with eviction
        stats.cache_misses++;
        stats.cache_evictions++;
    } else {// YA anything else we will count as error
        std::cerr << "[ERROR] Unexpected cache return code: "
                  << result << " for track \"" << track_name << "\"\n";
        stats.errors++;
    }

    //YA return the cache result code for the caller’s use
    return result;
}


/**
 * TODO: Implement load_track_to_mixer_deck method
 * 
 * @param track_title: Title of track to load to mixer
 * @return: Whether track was successfully loaded to a deck
 */
bool DJSession::load_track_to_mixer_deck(const std::string& track_title) {
    std::cout << "[System] Delegating track transfer to MixingEngineService for: "//YA information msg
              << track_title << std::endl;

    // YA bring the track from the cache
    AudioTrack* cached_track = controller_service.getTrackFromCache(track_title);

    // YA if the track it not in the cache
    if (!cached_track) {
        std::cerr << "[ERROR] Track: \"" << track_title
                  << "\" not found in cache\n";
        ++stats.errors;// YA increase the error counter
        return false; //YA didnt succeed to add it to the deck
    }

    // YA load the track to the deck by MixingEngine and take the track idx
    int deck_index = mixing_service.loadTrackToDeck(*cached_track);

    // YA checking the result
    //YA  0 -> deck A
    //YA  1 -> deck B
    //YA -1 -> error
    if (deck_index == 0) {
        ++stats.deck_loads_a;//YA update counter of deck a
        ++stats.transitions; // YA counter of loading everything to the deck
        return true;//YA succeeded
    }
    else if (deck_index == 1) {
        ++stats.deck_loads_b;//YA update counter of deck b
        ++stats.transitions;// YA counter of loading everything to the deck
        return true;//YA succeeded
    }
    else {//YA any result beside 0/1
        std::cerr << "[ERROR] Failed loading track: \"" << track_title
                  << "\" to any deck (return code " << deck_index << ")\n";
        ++stats.errors;
        return false;
    }
}


/**
 * @brief Main simulation loop that orchestrates the DJ performance session.
 * @note Updates session statistics (stats) throughout processing
 * @note Calls print_session_summary() to display results after playlist completion
 */
void DJSession::simulate_dj_performance() {
    std::cout << "=== DJ Controller System ===" << std::endl;
    std::cout << "Starting interactive DJ session..." << std::endl;

    // 1. Load configuration
    if (!load_configuration()) {
        std::cerr << "[ERROR] Failed to load configuration. Aborting session." << std::endl;
        return;
    }
    
    // 2. Build track library from config
    library_service.buildLibrary(session_config.library_tracks);
    
    // 3. Validate playlists exist
    if (session_config.playlists.empty()) {
        std::cerr << "[ERROR] No playlists found in configuration. Aborting session." << std::endl;
        return;
    }

    std::cout << "\nStarting DJ performance simulation..." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Capacity: " << session_config.controller_cache_size
              << " slots (LRU policy)" << std::endl;
    std::cout << "\n--- Processing Tracks ---" << std::endl;

    // YA sorted list of playlist names
    std::vector<std::string> playlist_names;
    playlist_names.reserve(session_config.playlists.size());
    for (const auto& pair : session_config.playlists) {
        playlist_names.push_back(pair.first);
    }
    std::sort(playlist_names.begin(), playlist_names.end());

    // YA all playlist status (A)
    if (play_all) {
        for (const std::string& pl_name : playlist_names) {
            std::cout << "\n[System] Auto-processing playlist: " << pl_name << "\n";

            //YA load the playlist to the library service
            if (!load_playlist(pl_name)) {
                std::cerr << "[ERROR] Failed to load playlist: " << pl_name << "\n";
                ++stats.errors;
                continue; // YA if we dont succeed, error and continue to the next playlist
            }

            std::cout << "[System] Playlist '" << pl_name
                      << "' loaded with " << track_titles.size() << " tracks.\n";//YA information msg

            // YA for all of the tracks in the playlist
            for (const std::string& title : track_titles) {
                std::cout << "\n-- Processing: " << title << " --\n";

                // YA counting every track we tried to proccess
                ++stats.tracks_processed;

                // YA load to cache and save the resulr
                int cache_result = load_track_to_controller(title);
                if (cache_result != 1 && cache_result != 0 && cache_result != -1) {
                    std::cerr << "[ERROR] Unexpected cache result (" << cache_result
                              << ") for track: " << title << "\n";
                    ++stats.errors;
                    continue;//YA every result that is not allowed-error and continue to the next
                }

                // YA load to deck and save the result true/false
                bool deck_ok = load_track_to_mixer_deck(title);
                if (!deck_ok) {
                    std::cerr << "[WARNING] Failed to load track '" << title
                              << "' to any deck. Skipping to next.\n";
                    continue;
                }
            }

            // YA summery of this playlist
            print_session_summary();
        }

        std::cout << "\n[System] All playlists processed.\n";
    }
    //YA iter status-default, play_all=false
    else {
        while (true) {
         //YA call the function you let us that let the user to choose the track
            std::string selected_playlist = display_playlist_menu_from_config();
            if (selected_playlist.empty()) {//YA empty choose=break
                std::cout << "[System] Session cancelled by user or no playlist selected.\n";
                break;
            }

            if (!load_playlist(selected_playlist)) {//YA try to load the choosen playlist
                std::cerr << "[ERROR] Failed to load playlist: " << selected_playlist << "\n";
                ++stats.errors;
                continue; 
            }

            std::cout << "[System] Playlist '" << selected_playlist
                      << "' loaded with " << track_titles.size() << " tracks.\n";//YA information msg

            for (const std::string& title : track_titles) {//YA for every track
                std::cout << "\n-- Processing: " << title << " --\n";

                ++stats.tracks_processed;//YA counting every track we tried to proccess

                int cache_result = load_track_to_controller(title);//YA the same proccess like (A)
                if (cache_result != 1 && cache_result != 0 && cache_result != -1) {
                    std::cerr << "[ERROR] Unexpected cache result (" << cache_result
                              << ") for track: " << title << "\n";
                    ++stats.errors;
                    continue;
                }
                
                // YA load to deck and save the result true/false
                bool deck_ok = load_track_to_mixer_deck(title);
                if (!deck_ok) {
                    std::cerr << "[WARNING] Failed to load track '" << title
                              << "' to any deck. Skipping to next.\n";
                    continue;
                }
            }

            print_session_summary();// YA summery of this playlist
        }
    }
}


/* 
 * Helper method to load session configuration from file
 * 
 * @return: true if configuration loaded successfully; false on error
 */
bool DJSession::load_configuration() {
    const std::string config_path = "bin/dj_config.txt";
    
    std::cout << "Loading configuration from: " << config_path << std::endl;
    
    if (!SessionFileParser::parse_config_file(config_path, session_config)) {
        std::cerr << "[ERROR] Failed to parse configuration file: " << config_path << std::endl;
        return false;
    }
    
    std::cout << "Configuration loaded successfully." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Size: " << session_config.controller_cache_size << " slots" << std::endl;
    mixing_service.set_auto_sync(session_config.auto_sync);
    mixing_service.set_bpm_tolerance(session_config.bpm_tolerance);
    //update cache size in LRUCache
    controller_service.set_cache_size(session_config.controller_cache_size);
    return true;
}

std::string DJSession::display_playlist_menu_from_config() {
    if (session_config.playlists.empty()) {
        return "";
    }
    
    std::cout << "\n=== Available Playlists ===" << std::endl;
    
    // Build sorted list of playlist names
    std::vector<std::string> playlist_names;
    for (const auto& pair : session_config.playlists) {
        playlist_names.push_back(pair.first);
    }
    std::sort(playlist_names.begin(), playlist_names.end());
    
    // Display numbered list
    for (size_t i = 0; i < playlist_names.size(); ++i) {
        std::cout << (i + 1) << ". " << playlist_names[i] << std::endl;
    }
    std::cout << "0. Cancel" << std::endl;
    
    // Prompt for user selection with validation
    int selection = -1;
    while (true) {
        std::cout << "\nSelect a playlist (1-" << playlist_names.size() << ", 0 to cancel): ";
        std::string input;
        
        if (!std::getline(std::cin, input)) {
            std::cout << "\n[ERROR] Input error. Cancelling session." << std::endl;
            return "";
        }
        
        std::stringstream ss(input);
        if (ss >> selection && ss.eof()) {
            if (selection == 0) {
                return "";
            } else if (selection >= 1 && selection <= static_cast<int>(playlist_names.size())) {
                std::string selected_name = playlist_names[selection - 1];
                std::cout << "Selected: " << selected_name << std::endl;
                return selected_name;
            }
        }
        
        std::cout << "Invalid selection. Please enter a number between 1 and " 
                  << playlist_names.size() << ", or 0 to cancel." << std::endl;
    }
}

void DJSession::print_session_summary() const {
    std::cout << "\n=== DJ Session Summary ===" << std::endl;
    std::cout << "Session: " << session_name << std::endl;
    std::cout << "Tracks processed: " << stats.tracks_processed << std::endl;
    std::cout << "Cache hits: " << stats.cache_hits << std::endl;
    std::cout << "Cache misses: " << stats.cache_misses << std::endl;
    std::cout << "Cache evictions: " << stats.cache_evictions << std::endl;
    std::cout << "Deck A loads: " << stats.deck_loads_a << std::endl;
    std::cout << "Deck B loads: " << stats.deck_loads_b << std::endl;
    std::cout << "Transitions: " << stats.transitions << std::endl;
    std::cout << "Errors: " << stats.errors << std::endl;
    std::cout << "=== Session Complete ===" << std::endl;
}