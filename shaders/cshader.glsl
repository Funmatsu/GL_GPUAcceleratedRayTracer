#version 430 core

#define PI 3.141592653589793238462643383279

struct ray{
	vec3 origin;
	vec3 direct;	
};

struct Sphere{
	vec4 center;
	vec4 ambtcol, diffcol, speccol;
	vec4 phongcoef;
	vec4 material;
};

struct Quad{
	vec4 p[4];
	vec4 ambtcol, diffcol, speccol;
	vec4 phongcoef;
	vec4 material;
};

struct Triad{
	vec4 p[3];
	vec4 ambtcol, diffcol, speccol;
	vec4 phongcoef;
	vec4 material;
};

struct Camera{
	vec4 forward;
	vec4 right;
	vec4 up;
	vec4 position;
};

struct PointLight{
	vec4 point;
};

struct AreaLight{
	vec4 p[4];
};

struct hitdata{
	int curshape;
	int type;
	float t;
	vec3 norm;
};

float intersectSphere(Sphere sp, ray r){
	vec3 oc = r.origin - vec3(sp.center);
	float b = 2.0f * dot(oc, r.direct), a = dot(r.direct, r.direct), c = dot(oc, oc) - sp.center.w*sp.center.w;
	// x = xo + t*xd, y = yo + t*yd, z = zo + t*zd in sphere equation (x - xc)^2 + (y - yc)^2 + (z - zc)^2 = r^2
	float disc = b * b - 4 * a * c;
	if(disc >= 0){
		float t1 = (-b + sqrt(disc)) / (2.0f * a),
			  t2 = (-b - sqrt(disc)) / (2.0f * a);
		float t = (t1 >= 0 && t2 >= 0) ? min(t1, t2) : max(t1, t2);
		return t;
	}
	else { return -1; }
}

float intersectQuad(Quad q, ray r) {
	vec3 normal = cross(vec3(q.p[1] - q.p[0]), vec3(q.p[2] - q.p[0]));
	float oa = dot(vec3(q.p[0]) - r.origin, normal);
	float vn = dot(r.direct, normal);
	
	if (abs(vn) < 1e-6) return -1;
	float t = oa/vn;
	vec3 point = r.origin + t * r.direct;
	vec3 newnorms[4];
	for (int i = 0; i < 4; i++) {
		vec3 edge = vec3(q.p[(i+1)%4] - q.p[i]);
		vec3 vp   = point - vec3(q.p[i]);
		newnorms[i] = cross(edge, vp);

		//newnorms[i] = cross(vec3(q.p[i] - q.p[(i + 1)%4]), vec3(q.p[i]) - point);
		newnorms[i] /= length(newnorms[i]);
		if (dot(newnorms[i], normal) < 0) return -1;
	}

	return t;
}

float intersectTriad(Triad q, ray r) {
	vec3 normal = cross(vec3(q.p[1] - q.p[0]), vec3(q.p[2] - q.p[0]));
	float oa = dot(vec3(q.p[0]) - r.origin, normal);
	float vn = dot(r.direct, normal);
	
	if (abs(vn) < 1e-6) return -1;
	float t = oa/vn;
	vec3 point = r.origin + t * r.direct;
	vec3 newnorms[4];
	for (int i = 0; i < 3; i++) {
		vec3 edge = vec3(q.p[(i+1)%3] - q.p[i]);
		vec3 vp   = point - vec3(q.p[i]);
		newnorms[i] = cross(edge, vp);

		//newnorms[i] = cross(vec3(q.p[i] - q.p[(i + 1)%4]), vec3(q.p[i]) - point);
		newnorms[i] /= length(newnorms[i]);
		if (dot(newnorms[i], normal) < 0) return -1;
	}

	return t;
}

layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba8, binding = 0) uniform writeonly image2D img;

layout(std430, binding = 1) buffer Spheres {
	Sphere spheres[];
};
layout(std430, binding = 2) buffer Quads {
	Quad quads[];
};
layout(std430, binding = 3) buffer Cameras {
	Camera cameras[];
};
layout(std430, binding = 4) buffer Triads {
	Triad triads[];
};
layout(std430, binding = 5) buffer PointLights {
	PointLight plights[];
};
layout(std430, binding = 6) buffer AreaLights {
	AreaLight alights[];
};

uniform int qdnum;
uniform int tdnum;
uniform int spnum;
uniform int plnum;
uniform int alnum;

float pcg(uint x) {
    x = x * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
    x = (x >> 22u) ^ x;
    return float(x) * (1.0 / 4294967296.0);
}

uint pcg_hash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float radicalInverse(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hamrand(uint i, uint N) {// hammersley for short i thinkß;
    return vec2(float(i) / float(N), radicalInverse(i));
}

float rand(inout uint state) {
	state = pcg_hash(state);
    return float(state) * (1.0 / 4294967296.0);
}

float rand01(vec2 co) {
	//return fract(sin(dot(co, pcg(uint(co.x)) * vec2(12.9898, 78.233))) * 43758.5453);
	//float r1 = pcg(9277u * uint(co.y) ^ (1973u * uint(co.x)));
	//return r1;
	//uint seed = uint(co.x) * 1973u ^ uint(co.y) * 9277u ^ uint(co.z) * 26699u;
	return 0.f;
}

vec3 getNormal(vec3 hitpos, int type, int curshape){
	if(type == 1)
		return normalize(cross(vec3(quads[curshape].p[0]) - vec3(quads[curshape].p[2]), vec3(quads[curshape].p[1]) - vec3(quads[curshape].p[2])));
	else if(type == 2)
		return normalize(cross(vec3(triads[curshape].p[0]) - hitpos, vec3(triads[curshape].p[1]) - hitpos));
	return -normalize(vec3(spheres[curshape].center) - hitpos);
}

vec3 brdf(uint seed, vec4 mat_type, ray r, vec3 norm){
	if(mat_type.x == 1){
		vec3 refldir = reflect(r.direct, norm);
		return refldir;
	}
	else if(mat_type.x == 2){
		float r1 = rand(seed),
			  r2 = rand(seed);

		float phi = 2 * PI * (r1),
			  costt = sqrt(1.f - r2),
			  sintt = sqrt(r2);
			  
		vec3 spheredir = vec3(cos(phi) * sintt, sin(phi) * sintt, costt);
		vec3 tang = abs(norm.x) > 0.1 ? vec3(0,1,0) : vec3(1,0,0);
		tang = normalize(cross(tang, norm));
		vec3 btan = (cross(norm, tang));

		vec3 hemidir = spheredir.x * tang + spheredir.y * btan + spheredir.z * norm;
		
		vec3 refldir = reflect(r.direct, norm);
		
		return normalize(mix(refldir, hemidir, mat_type.y));
	}
	else if(mat_type.x == 3){
		float normdir = dot(r.direct, norm);
		float eta = 1.f/mat_type.w;
		vec3 realnorm = norm;
		if(normdir > 0){
			realnorm = -norm;
			eta = 1/eta;
		}
		
		vec3 refldir = reflect(r.direct, norm);
		vec3 refrdir = refract(r.direct, realnorm, eta);
		float R0 = pow((1 - eta) / (1 + eta), 2.0);
		float Fresnel = R0 + (1.0 - R0) * pow(1.0 - clamp(normdir, -1.f, 1.f), 5.0);

		// Randomly choose reflection or refraction
		float r = rand(seed);
		return (r < 0.9) ? refrdir : refldir;
		//if(refrdir.x <= 0) return vec3(0, 1, -1);//reflect(r.direct, norm);
		return refrdir;
	}
	else {
		float r1 = rand(seed),
			  r2 = rand(seed);

		float phi = 2 * PI * (r1),
			  costt = sqrt(1.f - r2),
			  sintt = sqrt(r2);
			  
		vec3 spheredir = vec3(cos(phi) * sintt, sin(phi) * sintt, costt);
		vec3 tang = abs(norm.x) > 0.1 ? vec3(0,1,0) : vec3(1,0,0);
		tang = normalize(cross(tang, norm));
		vec3 btan = (cross(norm, tang));

		vec3 hemidir = spheredir.x * tang + spheredir.y * btan + spheredir.z * norm;

		return hemidir;
	}
}

bool occlude(ray lightRay, int type, float limdist){
	float t = -1;
	for (int k = 0; k < spnum; k++) {
		int kt = k;
		//while(spheres[kt].material.w == 3){
			t = intersectSphere(spheres[kt], lightRay);
			//vec3 hitpos = lightRay.origin + lightRay.direct * t;
			//ray tempray = ray(lightRay.origin, hitpos - lightRay.origin);
			//lightRay = ray(hitpos, brdf(0u, spheres[kt].material, tempray, getNormal(hitpos, 0, kt)));
			//
			//t = intersectSphere(spheres[kt], lightRay);
			//hitpos = lightRay.origin + lightRay.direct * t;
			//tempray = ray(lightRay.origin, hitpos - lightRay.origin);
			//lightRay = ray(hitpos, brdf(0u, spheres[kt].material, tempray, getNormal(hitpos, 0, kt)));
			
			//kt++;
		//}
		
		if (t > 1e-3 && t < limdist) {return true;}
	}	
	for (int k = 0; k < tdnum; k++) {
		int kt = k;
		//while(triads[k].material.w == 3){
			t = intersectTriad(triads[k], lightRay);
		//	vec3 hitpos = lightRay.origin + lightRay.direct * t;
		//	ray tempray = ray(lightRay.origin, hitpos - lightRay.origin);
		//	lightRay = ray(hitpos, brdf(0u, triads[k].material, tempray, getNormal(hitpos, 2, kt)));
		//	kt++;
		//}
		
		if (t > 1e-3 && t < limdist) {return true;}
	}	
	for (int k = 0; k < qdnum; k++) {
		int kt = k;
		//while(quads[kt].material.w == 3){
			t = intersectQuad(quads[kt], lightRay);
		//	vec3 hitpos = lightRay.origin + lightRay.direct * t;
		//	ray tempray = ray(lightRay.origin, hitpos - lightRay.origin);
		//	lightRay = ray(hitpos, brdf(0u, quads[kt].material, tempray, getNormal(hitpos, 1, kt)));
		//	kt++;
		//}
		
		if (t > 1e-3 && t < limdist) {return true;}
	}
			
	return false;
}

bool occlude(ray lightRay, int type, int curshape, float limdist){
	float t = -1;
	for (int k = 0; k < spnum; k++) {
		t = intersectSphere(spheres[k], lightRay);
		if (!(type == 0 && k == curshape) && t > 1e-3 && t < limdist) {return true;}
	}	
	for (int k = 0; k < tdnum; k++) {
		t = intersectTriad(triads[k], lightRay);
		if ((k != curshape) && t > 1e-3 && t < limdist) {return true;}
	}
	for (int k = 0; k < qdnum; k++) {
		t = intersectQuad(quads[k], lightRay);
		if (!(type == 1 && k == curshape) && t > 1e-3 && t < limdist) {return true;}
	}
			
	return false;
}

vec3 shadePoint(PointLight pl, ray r, Camera cam, int curShape, int type, float hit) {
	vec3 camPos = vec3(cam.position);
	vec3 hitpos = r.origin + hit * r.direct;
	vec3 pos = vec3(pl.point),
		 lightdir = (pos - hitpos);
	vec3 viewpos  = (camPos - hitpos);
	viewpos = normalize(viewpos);
	vec3 norm;
	int shapenum;
	if(type == 1) shapenum = qdnum;
	else if(type == 2) shapenum = tdnum;
	else shapenum = spnum;
	norm = getNormal(hitpos, type, curShape);
	norm = normalize(norm);

	float dist = length(lightdir);
	lightdir /= dist;

	ray lightRay = ray(hitpos + norm * 1e-3, lightdir);
	
	bool occluded = occlude(lightRay, type, dist);
			
	vec4 ambtcol, diffcol, speccol;
	float phong_coef;
	if(type == 1) {ambtcol = quads[curShape].ambtcol, diffcol = quads[curShape].diffcol, speccol = quads[curShape].speccol; phong_coef = quads[curShape].phongcoef.w;}
	else if(type == 2) {ambtcol = triads[curShape].ambtcol, diffcol = triads[curShape].diffcol, speccol = triads[curShape].speccol; phong_coef = triads[curShape].phongcoef.w;}
	else {ambtcol = spheres[curShape].ambtcol, diffcol = spheres[curShape].diffcol, speccol = spheres[curShape].speccol; phong_coef = spheres[curShape].phongcoef.w;}
	vec3 finalColor = vec3(ambtcol * ambtcol.w);
	if (!occluded) {
		vec3 halfDir = (lightdir + viewpos);
		vec3 reflec  = reflect(-lightdir, norm);
		halfDir /= length(halfDir);
		float diffFact = max(0.0f, dot(norm, lightdir));
		float specFact = pow(max(0.0f, dot(reflec, viewpos)), phong_coef);//pow(max(0.0f, dot(norm, halfDir)), phong_coef);

		vec3  //ambientColor  = shapes[curShape]->ambient_color * shapes[curShape]->ambient_coef,// *attenuation,///lightnum it was a parameter of this function;
			  diffuseColor  = vec3(diffcol * diffcol.w * diffFact),
			  specularColor = vec3(speccol * speccol.w * specFact);

		finalColor += (diffuseColor + specularColor);
	}

	return finalColor;
}

vec3 shadeArea(AreaLight al, ray r, Camera cam, int curShape, int type, float hit, uint seed){
	vec3 camPos = vec3(cam.position);
	vec3 hitpos = r.origin + hit * r.direct;
	vec3 u = vec3(al.p[1] - al.p[0]),
		 v = vec3(al.p[3] - al.p[0]),
		  viewpos = (camPos - hitpos);
	viewpos /= length(viewpos);

	float difffact = 0.f;
	float specfact = 0.f;
	float ambfact  = 0.f;
	int sqrn = int(al.p[0].w), n2 = sqrn;// * sqrn;
	
	vec4 ambtcol, diffcol, speccol;
	float phong_coef;
	if(type == 2) {ambtcol = triads[curShape].ambtcol, diffcol = triads[curShape].diffcol, speccol = triads[curShape].speccol; phong_coef = triads[curShape].phongcoef.w;}
	else if(type == 1) {ambtcol = quads[curShape].ambtcol, diffcol = quads[curShape].diffcol, speccol = quads[curShape].speccol; phong_coef = quads[curShape].phongcoef.w;}
	else {ambtcol = spheres[curShape].ambtcol, diffcol = spheres[curShape].diffcol, speccol = spheres[curShape].speccol; phong_coef = spheres[curShape].phongcoef.w;}
	
	//uint seed = uint(al.p[0].w) * 0xFu ^ uint(hitpos.x) * 0xFFu ^ uint(hitpos.y) * 0xFFFFu ^ uint(hitpos.z) * 0xFFFFFFFFu;
	
	for (float i = 0; i < n2; i++) {
			//float r1 = (i) / (n2);
			//float r2 = float(int(i) % sqrn) / (sqrn);
			//float r1 = hamrand(uint(i), uint(n2)).x;
			//float r2 = hamrand(uint(int(i) % sqrn), uint(n2)).y;
			float r1 = rand(seed);
			float r2 = rand(seed);

			vec3 point = vec3(al.p[0]) + u*(r1) + v*(r2);
			vec3 norm = getNormal(hitpos, type, curShape);
			norm /= length(norm);
			vec3 displacement = (point - hitpos);
			float dist = length(displacement);
			vec3 lightdir = displacement/dist;
			
			vec3 halfDir = (lightdir + viewpos);
			halfDir /= length(halfDir);
			ray lightRay = ray(hitpos + norm * 1e-3, lightdir);
			vec3 reflec = reflect(-lightdir, norm);//-lightdir - 2 * (dot(-lightdir, norm)) * norm;
			bool occluded = occlude(lightRay, type, dist);
			if (occluded == true) continue;

			ambfact += ambtcol.w;
			difffact += max(0.0f, dot(norm, lightdir));
			specfact += pow(max(0.0f, dot(reflec, viewpos)), phong_coef);
			//specfact += pow(max(0.0f, reflect.dot(viewpos)), shapes[curShape]->phong_coef);
	}

	ambfact /= n2;
	difffact /= n2;
	specfact /= n2;
	//float specfact = pow(max(0.0f, reflect.dot(viewpos)), shapes[curShape]->phong_coef);

	vec3 ambientColor = vec3(ambtcol)* ambfact,///; / lightnum was an orgument of this function
		  diffuseColor  = vec3(diffcol) * diffcol.w * difffact,
		  specularColor = vec3(speccol) * speccol.w * specfact;

	vec3 finalColor = ((ambientColor + diffuseColor + specularColor));

	return finalColor;
}

hitdata shoot(ray r){
	int curshape = -1, type = 0;
	float t = -1, tfront = -1;
	vec3 norm;
	for(int i = 0; i < spnum; i++){
		t = intersectSphere(spheres[i], r);
		if(t >= 0/*infront of camera*/ 
		  && ((t < tfront) || (tfront < 0))){ curshape = i; type = 0; tfront = t; norm = getNormal(r.origin + r.direct * t, type, i);}
	}
	for(int i = 0; i < qdnum; i++){
		t = intersectQuad(quads[i], r);
		if(t >= 0/*infront of camera*/ 
		  && ((t < tfront) || (tfront < 0))) { curshape = i; type = 1; tfront = t; norm = getNormal(r.origin + r.direct * t, type, i);}
	}
	for(int i = 0; i < tdnum; i++){
		t = intersectTriad(triads[i], r);
		if(t >= 0/*infront of camera*/ 
		  && ((t < tfront) || (tfront < 0))) { curshape = i; type = 2; tfront = t; norm = getNormal(r.origin + r.direct * t, type, i);}
	}
	
	return hitdata(curshape, type, tfront, norm);
}

vec3 getraycolor(ray r, vec3 hitpos, int curshape, int type){
	vec3 albedo;
	if(type == 2){
		albedo = triads[curshape].diffcol.xyz * triads[curshape].diffcol.w;
	}	
	if(type == 1){
		albedo = quads[curshape].diffcol.xyz * quads[curshape].diffcol.w;
	}	
	else if(type == 0){
		albedo = spheres[curshape].diffcol.xyz * spheres[curshape].diffcol.w;
	}
	
	float dist = distance(r.origin, hitpos);
	bool occluded = occlude(r, type, dist);
	if(occluded) return vec3(1);
	else return albedo;
}

vec3 shadeLights(float t, ray r, int curshape, int type, float samnum, uint seed){
	vec3 finalcolor = vec3(0);	
	vec3 samplpt;
	vec3 hitpos = r.origin + r.direct * t;
	vec3 norm = getNormal(hitpos, type, curshape);
	
	//for(int i = 0; i < alnum; i++){
	//	finalcolor += shadeArea(alights[i], r, cameras[0], curshape, type, t, seed);
	//}
	//
	//for(int i = 0; i < plnum; i++){
	//	finalcolor += shadePoint(plights[i], r, cameras[0], curshape, type, t);
	//}
	//
	//return finalcolor;
	
	vec4 ambtcol, diffcol, speccol;
	float phong_coef;
	if(type == 2) {ambtcol = triads[curshape].ambtcol, diffcol = triads[curshape].diffcol, speccol = triads[curshape].speccol; phong_coef = triads[curshape].phongcoef.w;}
	else if(type == 1) {ambtcol = quads[curshape].ambtcol, diffcol = quads[curshape].diffcol, speccol = quads[curshape].speccol; phong_coef = quads[curshape].phongcoef.w;}
	else {ambtcol = spheres[curshape].ambtcol, diffcol = spheres[curshape].diffcol, speccol = spheres[curshape].speccol; phong_coef = spheres[curshape].phongcoef.w;}
	
	float r1;
	float r2;
	float r3;
	
	for(int i = 0; i < spnum; i++){
		if(spheres[i].material.z <= 1) continue;
		
		int samsize = 4;
		vec3 col;
		for(int j = 0; j < samsize; j++){
			r1 = rand(seed);
			r2 = rand(seed);
			r3 = rand(seed);
			
			vec3 c = spheres[i].center.xyz;
			float r = spheres[i].center.w;
			
			samplpt = c + vec3(r*cos(2*PI*r1),
						   r*sin(2*PI*r2),
						   r*r3);
						   
			ray litray = ray(hitpos + norm * 1e-3, normalize(samplpt - hitpos));
			
			float dist = distance(samplpt, hitpos);
			bool occluded = occlude(litray, 0, i, dist);
			if(occluded == false) col += diffcol.xyz * diffcol.w * 0.5 * max(0.0, dot(norm, litray.direct));
		}
		col /= samsize;
		finalcolor += col;
	}
	
	for(int i = 0; i < tdnum; i++){
		if(triads[i].material.z <= 1) continue;
		
		r1 = rand(seed);
		r2 = rand(seed);
		
		vec3 u = (triads[i].p[1] - triads[i].p[0]).xyz,
			 v = (triads[i].p[2] - triads[i].p[0]).xyz;
		
		samplpt = triads[i].p[0].xyz + u * r1 + v * r2;
					   
		ray litray = ray(hitpos + norm * 1e-3, normalize(samplpt - hitpos));
		bool occluded = occlude(litray, 2, i, length(samplpt - hitpos));
		if(occluded == false) finalcolor += diffcol.xyz * diffcol.w;
	}
	
	for(int i = 0; i < qdnum; i++){
		if(quads[i].material.z <= 1) continue;
		int samsize = 4;
		vec3 col;
		for(int j = 0; j < samsize; j++){
			r1 = rand(seed);
			r2 = rand(seed);
			
			vec3 u = (quads[i].p[1] - quads[i].p[0]).xyz,
				 v = (quads[i].p[3] - quads[i].p[0]).xyz;
			
			samplpt = quads[i].p[0].xyz + u * r1 + v * r2;
						   
			ray litray = ray(hitpos + norm * 1e-3, normalize(samplpt - hitpos));
		
			float dist = distance(samplpt, hitpos);
			bool occluded = occlude(litray, 1, i, dist);
			if(occluded == false) col += diffcol.xyz * diffcol.w * 0.5 * max(0.0, dot(norm, litray.direct));
		}
		col /= samsize;
		finalcolor += col;
	}

	return finalcolor/2;
}

void main(){
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	
	Camera cam = cameras[0];
	imageStore(img, uv, vec4(0.5, 0.5, 0.5, 1));
	float h = tan((cam.position.w * PI/180)/2),
		  asp_rat = 1600.f / 900.f;
		  vec3 raydir;

	vec3 color = vec3(0);
	
	bool hit = false;
	float bouncenum = 4.f;
	int baseshape = -1;
	vec3 basecol = vec3(0);
	float samplenum = 2.f;
	
	for(int acc = 1; acc <= samplenum; acc++){
		//uint rayseed = uint(uv.x) * 1973u ^ uint(uv.y) * 9277u ^ uint(acc) * 26699u;
		vec2 rayxy = (uv + hamrand(uint(acc), uint(samplenum))) / vec2(1600, 900);
		raydir = (vec3(cam.forward) +
				  vec3(cam.right)   * (2*(rayxy.x) - 1) * h*asp_rat +
				  vec3(cam.up)      * (2*(rayxy.y) - 1) * h);
				   
		ray r;
		vec3 attfact = vec3(1);
		r.origin = vec3(cam.position);
		int curshape = -1, type;

		r.direct = raydir;
		for(int stage = 1; stage <= bouncenum; stage++){
			uint seed = uint(uv.x) * 1973u ^ uint(uv.y) * 9277u ^ uint(acc) * 26699u ^ uint(stage) * 1013904223u;

			hitdata hd = shoot(r);
			vec3 norm = hd.norm;
			curshape = hd.curshape, type = hd.type;
			float tfront = hd.t;
			
			//color = vec3(0);
			if(curshape >= 0){
				hit = true;
				vec3 newcol = vec3(0);
				vec3 albedo = vec3(1);
				
				float r1 = rand(seed),
					  r2 = rand(seed);
				
				vec3 dir;
				
				vec4 material;
				
				if(type == 0){ material = spheres[curshape].material; }
				else if(type == 1){ material = quads[curshape].material; }
				else if(type == 2){ material = triads[curshape].material; }
				
				dir = normalize(brdf(seed, material, r, norm));
				
				if(type == 2){
					albedo = triads[curshape].diffcol.xyz * triads[curshape].diffcol.w/2;
				}	
				else if(type == 1){
					albedo = quads[curshape].diffcol.xyz * quads[curshape].diffcol.w/2;
				}	
				else{
					albedo = spheres[curshape].diffcol.xyz * spheres[curshape].diffcol.w/2;
				}
				
				if(stage == 1){
					if(material.z > 1) { 
						color += vec3(1); 
						continue;
					}
					newcol += shadeLights(tfront, r, curshape, type, samplenum, seed);// + albedo * attfact;
					//color += newcol;
					//newcol += albedo * attfact;// * max(0.f, dot(norm, dir));
					color += newcol;// * material.z;
				}
				else {
					//float lumincoef = material.z;
					newcol += albedo * attfact;// * max(0.f, dot(norm, dir));
					color += newcol;// * material.z;
					//float p = max(attfact.r, max(attfact.g, attfact.b));
					//p = clamp(p, 0.1, 0.9);
					//
					//if (rand(seed) > p)
					//	break;
					//
					//attfact /= p;
				}
				
				attfact *= albedo;
				
				r.origin = (r.origin + r.direct * tfront);
				r.origin += dir * 1e-3;
				r.direct = dir;
			}
			if(tfront < 0){
				color += vec3(0);
			}
		}
	}
	
	if(hit == true)
		imageStore(img, uv, vec4((color/(samplenum)), 1));//vec3(r1, r2, rand01(vec2(r1, r2)))
}






//Imagine you are a proffessional graphics developper with years of experience in path tracing on the cpu and on the gpu. Please would you tell me how I can reduce the noise in sampling for some scenes given this compute shader I am using fill in a texture which I display on the screen in a glfw window.