#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, debug, space;

	std::vector<int> key_presses; // A to Z

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;

	//car honk sound:
	std::shared_ptr< Sound::PlayingSample > honk_oneshot;
	
	//camera:
	Scene::Camera *camera = nullptr;

	// ===== rhythm game stuff =======

	// preloaded image
	std::vector< glm::u8vec4 > alphabet_image;
	// objects
	Scene::Drawable *player = nullptr;
	Scene::Drawable *enemy = nullptr;
	Scene::Drawable *note_anchor = nullptr;
	Scene::Drawable *note_circle = nullptr;
	Scene::Drawable *lightning = nullptr;

	// The Note struct. defines a music note and its visual reference
	struct Note {
		// drawable
		Scene::Drawable *drawable;
		// #beats until landing.
		float life = 8.0f;
		// timer
		float hit_timer = 0.0f;
		// tint
		glm::vec4 tint;
		// result
		int result = 0;   // 0 = miss, 1 = bad, 2 = great, 3 = perfect
		// The letter to be shown
		char letter = ' ';
		bool consumed = false;
	};

	// List of all current notes
	std::list<Note> notes;

	// Method to add a new note
	void add_note(char letter);

	// BPM
	float bpm = 120.0f;
	// starting time
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	// current soundtrack time
	float curr_time = 0.0f;
	// most recent note timestamp
	float most_recent_note_time = -1.0f;
	// notes played
	size_t notes_played = 0;
	size_t current_round_played = 0;
	size_t word_index = 0;
	size_t current_round_bad = 0;
	size_t gameover = false;

	// Game statistics
	int num_bad = 0;
	int num_miss = 0;
	int num_great = 0;
	int num_perfect = 0;

	// aesthetics
	float press_timer = 0.0f;
	float lightning_timer = 0.0f;
	float enemy_fade = 1.0f;

	// base pos
	glm::vec3 player_base_pos;
	glm::vec3 enemy_base_pos;
};

const float CIRCLE_RADIUS = 2.0f;