#include "glShader.h"

glShader::glShader(){}
void glShader::createFromFiles(string vertexpath, string fragmentpath) {
	string vertexcode = parsefile(vertexpath); string fragmentcode = parsefile(fragmentpath);

	compileNlinkShader(vertexcode.c_str(), fragmentcode.c_str());
}
void glShader::createFromFiles(string computepath){
	string computecode = parsefile(computepath);

	compileNlinkShader(computecode.c_str());
}

string glShader::parsefile(string path){
	ifstream rdshader(path);
	if (!rdshader) {
		cout << "failed to open file at " << path << endl;
		return "";
	}
	string line;
	stringstream ss;
	while (getline(rdshader, line)) {
		ss << line;
		ss << "\n";
	}

	return ss.str();
}

unsigned int glShader::compile(GLenum type, const char* code) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	cout << code << endl;
	glCompileShader(shader);
	int ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		char errLog[512];
		glGetShaderInfoLog(shader, 512, NULL, errLog);
		cout << ((type == GL_VERTEX_SHADER) ? "Vertex shader " : (type == GL_FRAGMENT_SHADER) ? "Fragment Shader " : "Compute shader") << " error : \n" << errLog << endl;
	}
	return shader;
}

void glShader::compileNlinkShader(const char* vertexshader, const char* fragmentshader){
	unsigned int vsh = compile(GL_VERTEX_SHADER, vertexshader),
				 fsh = compile(GL_FRAGMENT_SHADER, fragmentshader);

	shid = glCreateProgram();
	glAttachShader(shid, vsh);
	glAttachShader(shid, fsh);
	
	glLinkProgram(shid);
	int ok;
	glGetProgramiv(shid, GL_LINK_STATUS, &ok);
	
	if (!ok) {
		char errLog[512];
		glGetProgramInfoLog(shid, 512, NULL, errLog);
		cout << "Linking unsuccessful: \n" << errLog << endl;
	}
	
	glDeleteShader(vsh);
	glDeleteShader(fsh);
}

void glShader::compileNlinkShader(const char* computeshader) {
	unsigned int csh = compile(GL_COMPUTE_SHADER, computeshader);

	shid = glCreateProgram();
	glAttachShader(shid, csh);

	glLinkProgram(shid);
	int ok;
	glGetProgramiv(shid, GL_LINK_STATUS, &ok);

	if (!ok) {
		char errLog[512];
		glGetProgramInfoLog(shid, 512, NULL, errLog);
		cout << "Linking unsuccessful: \n" << errLog << endl;
	}

	glDeleteShader(csh);
}

void glShader::use() {
	glUseProgram(shid);
}