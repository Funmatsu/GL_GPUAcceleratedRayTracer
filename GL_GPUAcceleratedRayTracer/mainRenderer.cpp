#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>

#include <glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>
#include <vector>
#include "Window.h"
#include "glShader.h"
#include "Mesh.h"
#include "glTexture.h"
#include "glGeometry.h"
#include "glSphere.h"
#include "glRectangle.h"
#include "ray_and_hit.h"
#include "glCamera.h"
#include "glLight.h"
#include "json.hpp"
#include <string>
#include <memory>

string vshader = "../shaders/vshader.glsl";
string fshader = "../shaders/fshader.glsl";
string cshader = "../shaders/cshader.glsl";
string scene   = "../assets/test_scene_me.json";// "../assets/knight.json";

using namespace std;
using json = nlohmann::json;

vector<glCamera> cameras;
vector<glSphere> spheres;
vector<glQuad> quads;
vector<glTriad> triads;
vector<glPointLight> lpoints;
vector<glAreaLight> lareas;

ostream& operator<<(ostream& os, vec2 v) {
	os << to_string(v);
	return os;
}

ostream& operator<<(ostream& os, vec3 v) {
	os << to_string(v);
	return os;
}

ostream& operator<<(ostream& os, vec4 v) {
	os << to_string(v);
	return os;
}

void parse_scenes(string path) {
	ifstream ifs(path.c_str());
	if (!ifs) {
		cout << path << " not found!" << endl;
		return;
	}
	cout << "parsing " << path.c_str() << endl;
	json j;
	stringstream ss;
	ss << ifs.rdbuf();
	j = json::parse(ss.str());
	for (auto itrs = j["output"].begin(); itrs != j["output"].end(); itrs++) {
		auto itrcam = *itrs;

		auto itc = itrcam["centre"],
			 itlook = itrcam["lookat"],
			 itup = itrcam["up"];
		
		//vec3f bkc = vec3f(itbkc[0], itbkc[1], itbkc[2]);
		//bool _2srender = itrcam.value("twosiderender", true);
		vec4 forward = vec4((itlook)[0],
							(itlook)[1],
							(itlook)[2], 1);
		vec4 up  = vec4((itup)[0],
						(itup)[1],
						(itup)[2], 1);
		vec4 right = vec4(cross(vec3(forward), vec3(up)), 1);
		vec4 position = vec4((itc)[0],
							 (itc)[1],
							 (itc)[2],
							 itrcam["fov"]);
		cameras.push_back(glCamera(
			forward,
			right,
			up,
			position
		));
		cout << cameras.back().up << " up" << endl;
	}
}

int parse_geometry(string path) {
	ifstream ifs(path.c_str());
	if (!ifs) {
		cout << path << " not found!" << endl;
		return 0;
	}
	cout << "parsing " << path.c_str() << endl;
	json j;
	stringstream ss;
	ss << ifs.rdbuf();
	j = json::parse(ss.str());

	std::cout << "Geometry: " << ss.str() << endl;
	int gc = 0;

	// use iterators to read-in array types
	for (auto itr = j["geometry"].begin(); itr != j["geometry"].end(); itr++) {
		cout << "p" << endl;
		std::string type;
		if (itr->contains("type")) {
			// type = static_cast<std::string>((*itr)["type"]);
			type = (*itr)["type"].get<std::string>();
		}
		else {
			std::cout << "Fatal error: geometry should always contain a type!!!" << endl;
			continue; // return 0
		}
		bool visible = (*itr).value("visible", true);
		vec4 specull_color = vec4((*itr)["sc"][0], (*itr)["sc"][1], (*itr)["sc"][2], (*itr)["ks"]);
		vec4 diffuse_color = vec4((*itr)["dc"][0], (*itr)["dc"][1], (*itr)["dc"][2], (*itr)["kd"]);
		vec4 ambient_color = vec4((*itr)["ac"][0], (*itr)["ac"][1], (*itr)["ac"][2], (*itr)["ka"]);
		if (visible) {
			cout << type << endl;
			if (type == "sphere") {
				std::cout << "Sphere: " << endl;
				vec3 center(0, 0, 0);
				int i = 0;
				for (auto itr2 = (*itr)["centre"].begin(); itr2 != (*itr)["centre"].end(); itr2++) {
					if (i < 3) {
						center[i++] = (*itr2).get<float>();
					}
					else {
						std::cout << "Warning: Too many entries in centre" << endl;
						continue;
					}
				}
				float radius = (*itr)["radius"];

				spheres.push_back(glSphere(center, radius, ambient_color, diffuse_color, specull_color));
				std::cout << "Centre: " << center << endl;

				spheres.back().phongcoef = vec4((*itr)["pc"].get<float>());
				spheres.back().material = vec4((*itr).value("material", 0), (*itr).value("roughness", 1.0f), (*itr).value("luminousity", 0.0f), (*itr).value("ior", 1.0f));
			}
			else if (type == "rectangle") {
				std::cout << "Rectangle: " << endl;
				vec4 p[4];

				for (int j = 0; j < 4; j++) {
					int i = 0;
					for (auto itr2 = (*itr)["p" + to_string(j + 1)].begin(); itr2 != (*itr)["p" + to_string(j + 1)].end(); itr2++) {

						if (i < 3) {
							p[j][i++] = (*itr2).get<float>();
						}
						else {
							std::cout << "Warning: Too many entries in centre" << endl;
							continue;
						}
					}
				}
				quads.push_back(glQuad(p, ambient_color, diffuse_color, specull_color));
				cout << "Points: " << p[0];
				cout << " " << p[1] << " " << p[2] << " " << p[3] << endl;

				quads.back().phongcoef = vec4((*itr)["pc"].get<float>());
				quads.back().material = vec4((*itr).value("material", 0), (*itr).value("roughness", 1.0f), (*itr).value("luminousity", 0.0f), (*itr).value("ior", 1.0f));
			}

			else if (type == "triangle") {
				std::cout << "Triangle: " << endl;
				vec4 p[3];

				for (int j = 0; j < 3; j++) {
					int i = 0;
					for (auto itr2 = (*itr)["p" + to_string(j + 1)].begin(); itr2 != (*itr)["p" + to_string(j + 1)].end(); itr2++) {

						if (i < 3) {
							p[j][i++] = (*itr2).get<float>();
						}
						else {
							std::cout << "Warning: Too many entries in centre" << endl;
							continue;
						}
					}
				}
				triads.push_back(glTriad(p, ambient_color, diffuse_color, specull_color));
				cout << "Points: " << p[0];
				cout << " " << p[1] << " " << p[2] << endl;

				triads.back().phongcoef = vec4((*itr)["pc"].get<float>());
				triads.back().material = vec4((*itr).value("material", 0), (*itr).value("roughness", 1.0f), (*itr).value("luminousity", 1.0f), (*itr).value("ior", 1.0f));
			}
			++gc;			
		}
	}

	std::cout << "We have: " << gc << " geometric objects!" << endl;
	return gc;
}

void parse_lights(string path) {
	ifstream ifs(path.c_str());
	if (!ifs) {
		cout << path << " not found!" << endl;
		return;
	}
	cout << "parsing " << path.c_str() << endl;
	json j;
	stringstream ss;
	ss << ifs.rdbuf();
	j = json::parse(ss.str());

	std::cout << "Light: " << endl;
	int lc = 0;

	// use iterators to read-in array types
	for (auto itr = j["light"].begin(); itr != j["light"].end(); itr++) {

		std::string type;
		if (itr->contains("type")) {
			//  type = static_cast<std::string>((*itr)["type"]);
			type = (*itr)["type"].get<std::string>();
		}
		else {
			std::cout << "Fatal error: light shoudl always contain a type!!!" << endl;
			return;
		}
		vec3 id = vec3((*itr)["id"][0],
			(*itr)["id"][1],
			(*itr)["id"][2]),
			is = vec3((*itr)["is"][0],
				(*itr)["is"][1],
				(*itr)["is"][2]);

		bool usecenter = (*itr).value("usecenter", false);
		if (type == "point") {// && ((*itr)["use"])
			if (!((*itr).contains("use") && !((*itr)["use"]))) {
				std::cout << "Point based light: " << endl;
				vec3 centre(0, 0, 0);
				int i = 0;
				for (auto itr2 = (*itr)["centre"].begin(); itr2 != (*itr)["centre"].end(); itr2++) {
					if (i < 3) {
						centre[i++] = (*itr2).get<float>();
					}
					else {
						std::cout << "Warning: Too many entries in centre" << endl;
					}
				}
				std::cout << "Centre: " << centre << endl;
				lpoints.push_back(glPointLight(centre));
			}
		}
		else if (type == "area") {
			if ((*itr).value("use", true)) {
				std::cout << "Area based light: " << endl;
				vec4 p[4];
				for (int j = 0; j < 4; j++) {
					int i = 0;
					for (auto itr2 = (*itr)["p" + to_string(j + 1)].begin(); itr2 != (*itr)["p" + to_string(j + 1)].end(); itr2++) {

						if (i < 3) {
							p[j][i++] = (*itr2).get<float>();
						}
						else {
							std::cout << "Warning: Too many entries in centre" << endl;
							continue;
						}
						p[j][3] = (*itr)["n"];
					}
				}
				std::cout << "light Points: " << p[0] << " " << p[1] << " " << p[2] << " " << p[3] << endl;
				if (usecenter) {
					vec3 center = vec3(p[0] + p[1] + p[2] + p[3]) / 4.f;
					cout << center << "Centerrrr" << endl;
					lpoints.push_back(glPointLight(center));
				}
				else {
					lareas.push_back(glAreaLight(p));
				}
			}
		}
		//lights.back()->dIntensity = id, lights.back()->sIntensity = is;
		++lc;
	}

	std::cout << "We have: " << lc << " objects!" << endl;

	return;
}

int main() {
	unsigned int spsbo, qdsbo, csbo, plsbo, alsbo, tdsbo; //sphere and quad storage buffer objects
	parse_scenes(scene);
	int gc = parse_geometry(scene);
	parse_lights(scene);
	Window mainWind(1600, 900, "the One");
	cout << "Hello My Beloved CPU!! GPU, you are next!" << endl;
	vector<float> verts{
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0,
		-1,  1, 0
	};

	vector<unsigned int> inds{
		0, 1, 2,
		2, 3, 0
	};
	Mesh m;
	m.sendMesh(verts, inds);
	glTexture gltx(1600, 900);
	glShader glsh, comsh;
	glsh.createFromFiles(vshader, fshader);
	comsh.createFromFiles(cshader);

	float frame_mult = 32;

	comsh.use();
	gltx.use();

	glUniform1i(glGetUniformLocation(comsh.getid(), "spnum"), spheres.size());
	glUniform1i(glGetUniformLocation(comsh.getid(), "qdnum"), quads.size());
	glUniform1i(glGetUniformLocation(comsh.getid(), "tdnum"), triads.size());
	glUniform1i(glGetUniformLocation(comsh.getid(), "plnum"), lpoints.size());
	glUniform1i(glGetUniformLocation(comsh.getid(), "alnum"), lareas.size());

	int shidx = 0;

	glGenBuffers(1, &spsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(glSphere), spheres.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, spsbo);

	glGenBuffers(1, &qdsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, qdsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, quads.size() * sizeof(glQuad), quads.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, qdsbo);

	glGenBuffers(1, &csbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, csbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, cameras.size() * sizeof(glCamera), cameras.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, csbo);

	glGenBuffers(1, &tdsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tdsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, triads.size() * sizeof(glTriad), triads.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, tdsbo);

	glGenBuffers(1, &plsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, plsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, lpoints.size() * sizeof(glQuad), lpoints.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, plsbo);
	
	glGenBuffers(1, &alsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, alsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, lareas.size() * sizeof(glCamera), lareas.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, alsbo);

	glDispatchCompute((1600)/frame_mult, (900)/frame_mult, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glsh.use();

	float aspect_ratio = 16.f / 9.f;
	mat4 model = translate(mat4(1), vec3(0, 0, -0.1)) * scale(mat4(1), vec3(aspect_ratio*9, 16/aspect_ratio, 2));
	mat4 proj = perspective(radians(45.f), aspect_ratio, 0.01f, 1000.f);
	glUniform1i(glGetUniformLocation(glsh.getid(), "text"), 0);
	glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "projmat"), 1, GL_FALSE, &proj[0][0]);

	gltx.bind();
	float step = 5*(1e-1)*gc;
	vec2 yawpitch = vec2(0);
	int rate = 0;
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, csbo);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, cameras.size() * sizeof(glCamera), cameras.data(), GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, csbo);

	//glDispatchCompute(frame_mult * (1600) / 32, frame_mult * (900) / 32, 1);
	//comsh.use();
	//gltx.use();
	//gltx.bind();
	while (!mainWind.shouldClose()) {
		
		bool pressed = 0;
		glfwPollEvents();
		auto start = chrono::high_resolution_clock::now();
		mainWind.swapNpoll();
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		if (mainWind.pressed(GLFW_KEY_W)) {
			pressed = 1;
			//model = translate(model, vec3(0, 0, -1e-2));
			//glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);

			cameras[0].position += step * vec4(cameras[0].forward.x, 0, cameras[0].forward.z, 0);
		}
		if (mainWind.pressed(GLFW_KEY_S)) {
			pressed = 1;
			//model = translate(model, vec3(0, 0, -1e-2));
			//glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);

			cameras[0].position -= step * vec4(cameras[0].forward.x, 0, cameras[0].forward.z, 0);
		}
		if (mainWind.pressed(GLFW_KEY_A)) {
			pressed = 1;
			//model = translate(model, vec3(0, 0, -1e-2));
			//glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);

			cameras[0].position -= step * vec4(vec3(cameras[0].right), 0);
		}
		if (mainWind.pressed(GLFW_KEY_D)) {
			pressed = 1;
			//model = translate(model, vec3(0, 0, -1e-2)); 
			//glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);

			cameras[0].position += step * vec4(vec3(cameras[0].right), 0);
		}
		if (mainWind.pressed(GLFW_KEY_ENTER) || mainWind.pressed(GLFW_KEY_LEFT_SHIFT)) {
			pressed = 1;
			//model = translate(model, vec3(0, 0, -1e-2));
			//glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);

			cameras[0].position.y -= step;
		}
		if (mainWind.pressed(GLFW_KEY_SPACE)) {
			pressed = 1;
			//model = translate(model, vec3(0, 0, -1e-2));
			//glUniformMatrix4fv(glGetUniformLocation(glsh.getid(), "modlmat"), 1, GL_FALSE, &model[0][0]);

			cameras[0].position.y += step;
		}

		if (pressed || mainWind.getMouseMoved()) {
			if (mainWind.getMouseMoved()) {
				//cout << cameras[0].forward << endl;
				vec3 forward = cameras[0].getForwardVec(yawpitch, mainWind.getMouseChange()),
					right = cross(vec3(forward), vec3(vec3(0, 1, 0))),
					up = cross(vec3(right), vec3(forward));
				cameras[0].forward = vec4(normalize(forward), 1);
				cameras[0].right = vec4(normalize(right), 1);
				cameras[0].up = vec4(normalize(up), 1);
			}

			comsh.use();
			gltx.use();
			
			//if(rate++ % 5){
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, csbo);
				glBufferData(GL_SHADER_STORAGE_BUFFER, cameras.size() * sizeof(glCamera), cameras.data(), GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ++shidx, csbo);
				glDispatchCompute((1600) / frame_mult, (900) / frame_mult, 1);
			//}
				auto end = chrono::high_resolution_clock::now();
				cout << chrono::duration<double>(end - start).count() << endl;
			
			glFinish();
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glsh.use();
			gltx.bind();
		}
		m.drawMesh();
		//auto end = chrono::high_resolution_clock::now();
		////cout << chrono::duration<double>(end - start).count() << endl;
	}
}

void unlock() {
	bool input = 0;
	bool c = 0, R = 0;
	int state = 0;
	unsigned int code = 256;//101101 32+13
	int bitnum = log(code) / log(2);
	while (state <= bitnum) {
		cin >> c;
		cin >> R;
		if (R) state = 0; // first digit is the reset bit=

		if (c == (code >> (bitnum - state) & 1)) {
			cout << "state was " << state << endl;
			state++;
		}
		else if (c == 1) {
			state = 1;
			cout << "state was 1 tho" << endl;
		}
		else {
			state = 0;
			cout << "nth" << endl;
		}
	}
	printf("unlocked!\n");

}