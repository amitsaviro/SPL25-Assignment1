#include "MixingEngineService.h"
#include "PointerWrapper.h"
#include <iostream>
#include <memory>

//YA set this class to be the ownership of AudioTrack
/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : decks{nullptr, nullptr}, // YA both deacks are empty
      active_deck(0),  // YA deck 0 is the first one but still empty
      auto_sync(false), // the auto bpm sync is getting false for deafult 
      bpm_tolerance(0)  // dont allow and tolerance yet
{
    std::cout << "[MixingEngineService] Initialized with 2 empty decks\n"; //YA cout msg
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    std::cout << "[MixingEngineService] Cleaning up decks...\n"; //YA cleaning msg

    for (AudioTrack*& deck : decks) { // YA checking both
        if (deck != nullptr) {
            delete deck;   //YA delete the Track of this deck-deck is the ownership of the AudioTrack
            deck = nullptr;    // YA set the ptr to null
        }
    }
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    std::cout << "\n=== Loading Track to Deck ===\n";//YA loading msg

    //YA clone the track
    PointerWrapper<AudioTrack> clone(track.clone());
    if (!clone) {
        std::cerr << "[ERROR] Track: \"" << track.get_title()//error msg if clone is fail
                  << "\" failed to clone\n";
        return -1;
    }

    //YA bool flag if this is the first track ever loaded
    bool first_load = (decks[0] == nullptr && decks[1] == nullptr);
    size_t target = first_load ? 0 : (1 - active_deck);// target deck-if it is the first: 0, else: inactive deck
    std::cout << "[Deck] Target deck: " << target << "\n";

    //YA if target deck is occupied, delete old track
    if (decks[target] != nullptr) {
        //YA checking msg
        std::string old_title = decks[target]->get_title();
        std::cout << "[Unload] Clearing previous track from deck "
                  << target << ": " << old_title << "\n";
        delete decks[target];//YA delete track from memory
        decks[target] = nullptr;//YA deck is empty
    }
    //YA prepare track for playback
    clone->load();
    clone->analyze_beatgrid();

    if (!first_load && auto_sync) {//YA is it isnt the first and auto_sync on
        if (!can_mix_tracks(clone)) {//YA cant mix tracks-high tolerance
            sync_bpm(clone);
        }
    }

    //YA transfer ownership from clone to the deck
    AudioTrack* prepared = clone.release();  // YA clone isnt the ownership
    decks[target] = prepared;          // YA deck is the owner
    std::cout << "[Load Complete] \"" << prepared->get_title()
              << "\" is now loaded on deck " << target << "\n";

    //YA unload the previous active deck
    if (!first_load && active_deck != target && decks[active_deck] != nullptr) {//YA if the target one isnt the active one
        std::string old_active_title = decks[active_deck]->get_title();
        std::cout << "[Unload] Unloading previous active deck "
                  << active_deck << ": " << old_active_title << "\n";

        delete decks[active_deck];//YA delete the active
        decks[active_deck] = nullptr;//YA set deck to null
    }

    //YA update active deck + return
    active_deck = target;
    std::cout << "[Active Deck] Switched to deck " << active_deck << "\n";
    return static_cast<int>(target);
}


/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const 
{
    if (!track || decks[active_deck] == nullptr) { //YA if there is no active deck or track we cant mix-false
        return false;
    }

    int active_bpm   = decks[active_deck]->get_bpm();  // YA BPM of the active deck
    int incoming_bpm = track->get_bpm();               // YA BPM of the new track

    int diff = active_bpm - incoming_bpm;//YA difference
    if (diff < 0) {
        diff = -diff; //YA abs
    }

    //YA if the diff <= tolerance return true
    return diff <= bpm_tolerance;
}


/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const 
{
    if (!track || decks[active_deck] == nullptr) {//YA if there is no active deck or track we cant sync
        return;
    }

    int active_bpm   = decks[active_deck]->get_bpm();  // YA BPM of the active deck
    int incoming_bpm = track->get_bpm();               // YA BPM of the new track

    int avg_bpm = (active_bpm + incoming_bpm) / 2; //YA AVG calc

    track->set_bpm(avg_bpm);//YA set the new BPM
}

