#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"

#include "Chart.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

constexpr float PI = 3.1415926535897932384626f;

// My scenes
GLuint cave_program = 0;
Load< MeshBuffer > cave_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("cave.pnct"));
	cave_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > cave_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("cave.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = cave_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = cave_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// need a default uniform so other pieces don't get overwritten
		drawable.pipeline.set_uniforms = []() {
			glUniform4fv(lit_color_texture_program->TINT_vec4, 1, glm::value_ptr(glm::vec4(1.0f)));
		};

		// For Player and Enemy, we want to change the texture
		if (Chart::mesh_name_to_sprite.find(mesh_name) != Chart::mesh_name_to_sprite.end()) {
			GLuint tex;
			glGenTextures(1, &tex);

			glBindTexture(GL_TEXTURE_2D, tex);
			std::vector< glm::u8vec4 > tex_data(0);
			glm::uvec2 loc({256, 256});

			load_png(data_path(Chart::mesh_name_to_sprite[mesh_name]), &loc, &tex_data, LowerLeftOrigin);

			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA,
				Chart::mesh_name_to_width[mesh_name], Chart::mesh_name_to_width[mesh_name],
				0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);

			drawable.pipeline.textures[0].texture = tex;
			drawable.pipeline.textures[0].target = GL_TEXTURE_2D;
			drawable.blended = true;
		}

	});
});



Load< Sound::Sample > song_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("song.opus"));
});


Load< Sound::Sample > honk_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("honk.wav"));
});

Load< Sound::Sample > sfx_perfect_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("sfx_perfect.opus"));
});

Load< Sound::Sample > sfx_great_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("sfx_great.opus"));
});

Load< Sound::Sample > sfx_bad_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("sfx_bad.opus"));
});

PlayMode::PlayMode() : scene(*cave_scene) {

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// get pointers to some objects for convenience:
	for (auto &drawable : scene.drawables) {
		if (drawable.transform->name == "Player") player = &drawable;
		else if (drawable.transform->name == "Enemy") enemy = &drawable;
		else if (drawable.transform->name == "Anchor") note_anchor = &drawable;
	}
	if (player == nullptr) throw std::runtime_error("Player not found.");
	if (enemy == nullptr) throw std::runtime_error("Enemy not found.");
	if (note_anchor == nullptr) throw std::runtime_error("Anchor not found.");

	//start music loop playing:
	// (note: position will be over-ridden in update())
	leg_tip_loop = Sound::loop_3D(*song_sample, 1.0f, glm::vec3(0.0f), 10.0f);

	// Load image
	glm::uvec2 loc({512, 256});
	load_png(data_path("Alphabet.png"), &loc, &alphabet_image, LowerLeftOrigin);

	// Initialize game states
	key_presses = std::vector<int>(26);
	for (auto &v: key_presses) {
		v = 0;
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {

		// Update based on key name
		switch (evt.key.key) {
			case SDLK_A: key_presses[0] += 1; break;
			case SDLK_B: key_presses[1] += 1; break;
			case SDLK_C: key_presses[2] += 1; break;
			case SDLK_D: key_presses[3] += 1; break;
			case SDLK_E: key_presses[4] += 1; break;
			case SDLK_F: key_presses[5] += 1; break;
			case SDLK_G: key_presses[6] += 1; break;
			case SDLK_H: key_presses[7] += 1; break;
			case SDLK_I: key_presses[8] += 1; break;
			case SDLK_J: key_presses[9] += 1; break;
			case SDLK_K: key_presses[10] += 1; break;
			case SDLK_L: key_presses[11] += 1; break;
			case SDLK_M: key_presses[12] += 1; break;
			case SDLK_N: key_presses[13] += 1; break;
			case SDLK_O: key_presses[14] += 1; break;
			case SDLK_P: key_presses[15] += 1; break;
			case SDLK_Q: key_presses[16] += 1; break;
			case SDLK_R: key_presses[17] += 1; break;
			case SDLK_S: key_presses[18] += 1; break;
			case SDLK_T: key_presses[19] += 1; break;
			case SDLK_U: key_presses[20] += 1; break;
			case SDLK_V: key_presses[21] += 1; break;
			case SDLK_W: key_presses[22] += 1; break;
			case SDLK_X: key_presses[23] += 1; break;
			case SDLK_Y: key_presses[24] += 1; break;
			case SDLK_Z: key_presses[25] += 1; break;
		}

		if (evt.key.key == SDLK_ESCAPE) {
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		} else if (evt.key.key == SDLK_A) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			if (honk_oneshot) honk_oneshot->stop();
			honk_oneshot = Sound::play_3D(*honk_sample, 0.3f, glm::vec3(4.6f, -7.8f, 6.9f)); //hardcoded position of front of car, from blender
			space.downs += 1;
			space.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_0) {
			debug.downs += 1;
			debug.pressed += true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			space.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_0) {
			debug.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

	//move sound to follow leg tip position:
	// TODO

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_parent_from_local();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}


	{ // DEBUG: spawn a note on debug button
		if (debug.downs > 0) {
			add_note('a');
		}
	}

	{ //Handle note positions and note hantei
		Note *next_note = nullptr;
		for (Note &note: notes) {
			// proceed, assuming no game lag
			note.life -= elapsed * bpm / 60.0f;

			// find next note
			if (note.life > -0.25f && note.life < 2.0f && !note.consumed) {
				if (next_note == nullptr) {
					next_note = &note;
				} else if (next_note->life > note.life) {
					next_note = &note;
				}
			}

			// Change position based on note life
			if (note.life > -0.3f && !note.consumed) {
				note.drawable->transform->position.y = -glm::cos(note.life / 4.0f * PI) * CIRCLE_RADIUS;
				note.drawable->transform->position.z = -glm::sin(note.life / 4.0f * PI) * CIRCLE_RADIUS;
			}
			// Change note rendering style based on note life
			if (note.life > 4.0f) {
				note.drawable->transform->scale = glm::vec3(0.28f);
			} else if (note.life > 1.0f) {
				note.drawable->transform->scale = glm::vec3(0.44f - 0.04f * note.life);
			} else {
				note.drawable->transform->scale = glm::vec3(0.4f);
			}

			// Change coloring based on note result, if consumed
			if (note.consumed) {
				note.hit_timer += elapsed;
				note.tint.a = std::clamp(1.0f - 3.0f * note.hit_timer, 0.0f, 1.0f);
			}
		}
		// If there is a note that can be consumed
		if (next_note != nullptr) {
			for (char letter = 'a'; letter <= 'z'; letter++) {
				// If the key has been pressed
				if (key_presses[letter - 'a'] > 0) {
					next_note->consumed = true;
					// Check the correctness, and timing.
					bool is_key_correct = next_note->letter == letter;
					float time_diff = next_note->life * 60.0f / bpm;
					
					if (!is_key_correct) {
						next_note->result = 0;
						next_note->tint.g = 0.3f;
						next_note->tint.b = 0.3f;
						next_note->tint.r = 0.3f;
						if (honk_oneshot) honk_oneshot->stop();
						honk_oneshot = Sound::play_3D(*sfx_bad_sample, 0.6f, glm::vec3(0.0f, 0.0f, 0.0f));
						num_miss++;
					} else if (time_diff < 0.08f && time_diff > -0.06f) { // HANTEI
						next_note->tint.b = 0.5f;
						next_note->result = 3;
						if (honk_oneshot) honk_oneshot->stop();
						honk_oneshot = Sound::play_3D(*sfx_perfect_sample, 0.6f, glm::vec3(0.0f, 0.0f, 0.0f));
						num_perfect++;
					} else if (time_diff < 0.12f && time_diff > -0.10f) {
						next_note->tint.r = 0.4f;
						next_note->tint.g = 0.8f;
						next_note->result = 2;
						if (honk_oneshot) honk_oneshot->stop();
						honk_oneshot = Sound::play_3D(*sfx_great_sample, 0.6f, glm::vec3(0.0f, 0.0f, 0.0f));
						num_great++;
					} else {
						next_note->tint.g = 0.4f;
						next_note->tint.b = 0.4f;
						next_note->result = 1;
						if (honk_oneshot) honk_oneshot->stop();
						honk_oneshot = Sound::play_3D(*sfx_bad_sample, 0.6f, glm::vec3(0.0f, 0.0f, 0.0f));
						num_bad++;
					}
					break;
				}
			}
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
	debug.downs = 0;

	// reset more counters:
	key_presses = std::vector<int>(26);
	for (auto &v: key_presses) {
		v = 0;
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_world_from_local() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}

void PlayMode::add_note(char letter) {
	// get Plane mesh
	Mesh const &mesh = cave_meshes->lookup("Plane");

	// add a new transform
	scene.transforms.emplace_back();
	Scene::Transform &xform = scene.transforms.back();

	// Complete the transform and drawable init
	xform.name = "Note_" + std::to_string(notes_played);
	xform.parent = note_anchor->transform;
	xform.position = glm::vec3(0.0f);
	xform.scale = glm::vec3(0.4f, 0.4f, 0.4f);
	xform.rotation = glm::quat(glm::vec3{PI/2, 0.0f, PI/2});

	scene.drawables.emplace_back(&xform);
	Scene::Drawable &drawable = scene.drawables.back();
	drawable.pipeline = lit_color_texture_program_pipeline;
	drawable.pipeline.vao = cave_program;
	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;
	drawable.blended = true;

	// Load texture
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	std::vector<glm::u8vec4> subimage(64 * 64);
	{
		size_t by = 3 - (letter - 'a') / 8;     // top-left coordinate
		size_t bx = (letter - 'a') % 8;
		for (size_t dy = 0; dy < 64; ++dy) {
			glm::u8vec4 const *src = alphabet_image.data() + (by * 64 + dy) * 512 + (bx * 64);
			glm::u8vec4 *dest = subimage.data() + dy * 64;
			std::memcpy(dest, src, 64 * sizeof(glm::u8vec4));
		}
	}

	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, alphabet_image.data());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, subimage.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	drawable.pipeline.textures[0].texture = tex;
	drawable.pipeline.textures[0].target = GL_TEXTURE_2D;

	notes.emplace_back();
	Note *note = &notes.back();
	static_cast<void>(note);
	notes.back().drawable = &drawable;
	notes.back().letter = letter;
	notes.back().life = 8.0f;
	notes.back().tint = glm::u8vec4{1.0f, 1.0f, 1.0f, 1.0f};

	// Set uniforms to modulate color
	drawable.pipeline.set_uniforms = [note]() {
		glUniform4fv(lit_color_texture_program->TINT_vec4, 1, glm::value_ptr(note->tint));
	};

}