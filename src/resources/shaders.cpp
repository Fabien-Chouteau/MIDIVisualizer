#include "data.h"
const std::map<std::string, std::string> shaders = {
{ "background_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "background_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform float time;\n uniform float secondsPerMeasure;\n uniform vec2 inverseScreenSize;\n uniform bool useDigits = true;\n uniform bool useHLines = true;\n uniform bool useVLines = true;\n uniform float minorsWidth = 1.0;\n uniform sampler2D screenTexture;\n uniform vec3 textColor = vec3(1.0);\n uniform vec3 linesColor = vec3(1.0);\n #define MAJOR_COUNT 75.0\n const float octaveLinesPositions[11] = float[](0.0/75.0, 7.0/75.0, 14.0/75.0, 21.0/75.0, 28.0/75.0, 35.0/75.0, 42.0/75.0, 49.0/75.0, 56.0/75.0, 63.0/75.0, 70.0/75.0);\n 			\n uniform float mainSpeed;\n uniform float keyboardHeight = 0.25;\n uniform int minNoteMajor;\n uniform float notesCount;\n out vec4 fragColor;\n float printDigit(int digit, vec2 uv){\n 	// Clamping to avoid artifacts.\n 	if(uv.x < 0.01 || uv.x > 0.99 || uv.y < 0.01 || uv.y > 0.99){\n 		return 0.0;\n 	}\n 	\n 	// UV from [0,1] to local tile frame.\n 	vec2 localUV = uv * vec2(50.0/256.0,0.5);\n 	// Select the digit.\n 	vec2 globalUV = vec2( mod(digit,5)*50.0/256.0,digit < 5 ? 0.5 : 0.0);\n 	// Combine global and local shifts.\n 	vec2 finalUV = globalUV + localUV;\n 	\n 	// Read from font atlas. Return if above a threshold.\n 	float isIn = texture(screenTexture, finalUV).r;\n 	return isIn < 0.5 ? 0.0 : isIn ;\n 	\n }\n float printNumber(float num, vec2 position, vec2 uv, vec2 scale){\n 	if(num < -0.1){\n 		return 0.0f;\n 	}\n 	if(position.y > 1.0 || position.y < 0.0){\n 		return 0.0;\n 	}\n 	\n 	// We limit to the [0,999] range.\n 	float number = min(999.0, max(0.0,num));\n 	\n 	// Extract digits.\n 	int hundredDigit = int(floor( number / 100.0 ));\n 	int tenDigit	 = int(floor( number / 10.0 - hundredDigit * 10.0));\n 	int unitDigit	 = int(floor( number - hundredDigit * 100.0 - tenDigit * 10.0));\n 	\n 	// Position of the text.\n 	vec2 initialPos = scale*(uv-position);\n 	\n 	// Get intensity for each digit at the current fragment.\n 	float hundred = printDigit(hundredDigit, initialPos);\n 	float ten	  =	printDigit(tenDigit,	 initialPos - vec2(scale.x * 0.009,0.0));\n 	float unit	  = printDigit(unitDigit,	 initialPos - vec2(scale.x * 0.009 * 2.0,0.0));\n 	\n 	// If hundred digit == 0, hide it.\n 	float hundredVisibility = (1.0-step(float(hundredDigit),0.5));\n 	hundred *= hundredVisibility;\n 	// If ten digit == 0 and hundred digit == 0, hide ten.\n 	float tenVisibility = max(hundredVisibility,(1.0-step(float(tenDigit),0.5)));\n 	ten*= tenVisibility;\n 	\n 	return hundred + ten + unit;\n }\n void main(){\n 	\n 	vec4 bgColor = vec4(0.0);\n 	// Octaves lines.\n 	if(useVLines){\n 		// send 0 to (minNote)/MAJOR_COUNT\n 		// send 1 to (maxNote)/MAJOR_COUNT\n 		float a = (notesCount) / MAJOR_COUNT;\n 		float b = float(minNoteMajor) / MAJOR_COUNT;\n 		float refPos = a * In.uv.x + b;\n 		for(int i = 0; i < 11; i++){\n 			float linePos = octaveLinesPositions[i];\n 			float lineIntensity = 0.7 * step(abs(refPos - linePos), inverseScreenSize.x / MAJOR_COUNT * notesCount);\n 			bgColor = mix(bgColor, vec4(linesColor, 1.0), lineIntensity);\n 		}\n 	}\n 	\n 	vec2 scale = 1.5*vec2(64.0,50.0*inverseScreenSize.x/inverseScreenSize.y);\n 	\n 	// Text on the side.\n 	int currentMesure = int(floor(time/secondsPerMeasure));\n 	// How many mesures do we check.\n 	int count = int(ceil(0.75*(2.0/mainSpeed)))+2;\n 	\n 	for(int i = 0; i < count; i++){\n 		// Compute position of the measure currentMesure+i.\n 		vec2 position = vec2(0.005, keyboardHeight + (secondsPerMeasure*(currentMesure+i) - time)*mainSpeed*0.5);\n 		\n 		// Compute color for the number display, and for the horizontal line.\n 		float numberIntensity = useDigits ? printNumber(currentMesure + i,position, In.uv, scale) : 0.0;\n 		bgColor = mix(bgColor, vec4(textColor, 1.0), numberIntensity);\n 		float lineIntensity = useHLines ? (0.25*(step(abs(In.uv.y - position.y - 0.5 / scale.y), inverseScreenSize.y))) : 0.0;\n 		bgColor = mix(bgColor, vec4(linesColor, 1.0), lineIntensity);\n 	}\n 	\n 	if(all(equal(bgColor.xyz, vec3(0.0)))){\n 		// Transparent background.\n 		discard;\n 	}\n 	\n 	fragColor = bgColor;\n 	\n }\n "},
{ "flashes_vert", "#version 330\n layout(location = 0) in vec2 v;\n layout(location = 1) in int onChan;\n uniform float time;\n uniform vec2 inverseScreenSize;\n uniform float userScale = 1.0;\n uniform float keyboardHeight = 0.25;\n uniform int minNote;\n uniform float notesCount;\n const float shifts[128] = float[](\n 	0,0.5,1,1.5,2,3,3.5,4,4.5,5,5.5,6,7,7.5,8,8.5,9,10,10.5,11,11.5,12,12.5,13,14,14.5,15,15.5,16,17,17.5,18,18.5,19,19.5,20,21,21.5,22,22.5,23,24,24.5,25,25.5,26,26.5,27,28,28.5,29,29.5,30,31,31.5,32,32.5,33,33.5,34,35,35.5,36,36.5,37,38,38.5,39,39.5,40,40.5,41,42,42.5,43,43.5,44,45,45.5,46,46.5,47,47.5,48,49,49.5,50,50.5,51,52,52.5,53,53.5,54,54.5,55,56,56.5,57,57.5,58,59,59.5,60,60.5,61,61.5,62,63,63.5,64,64.5,65,66,66.5,67,67.5,68,68.5,69,70,70.5,71,71.5,72,73,73.5,74\n );\n const vec2 scale = 0.9*vec2(3.5,3.0);\n out INTERFACE {\n 	vec2 uv;\n 	float onChannel;\n 	float id;\n } Out;\n void main(){\n 	\n 	// Scale quad, keep the square ratio.\n 	vec2 scaledPosition = v * 2.0 * scale * userScale/notesCount * vec2(1.0, inverseScreenSize.y/inverseScreenSize.x);\n 	// Shift based on note/flash id.\n 	vec2 globalShift = vec2(-1.0 + ((shifts[gl_InstanceID] - shifts[minNote]) * 2.0 + 1.0) / notesCount, 2.0 * keyboardHeight - 1.0);\n 	\n 	gl_Position = vec4(scaledPosition + globalShift, 0.0 , 1.0) ;\n 	\n 	// Pass infos to the fragment shader.\n 	Out.uv = v;\n 	Out.onChannel = float(onChan);\n 	Out.id = float(gl_InstanceID);\n 	\n }\n "}, 
{ "flashes_frag", "#version 330\n #define CHANNELS_COUNT 8\n in INTERFACE {\n 	vec2 uv;\n 	float onChannel;\n 	float id;\n } In;\n uniform sampler2D textureFlash;\n uniform float time;\n uniform vec3 baseColor[CHANNELS_COUNT];\n #define numberSprites 8.0\n out vec4 fragColor;\n float rand(vec2 co){\n 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n }\n void main(){\n 	\n 	// If not on, discard flash immediatly.\n 	int cid = int(In.onChannel);\n 	if(cid < 0){\n 		discard;\n 	}\n 	float mask = 0.0;\n 	\n 	// If up half, read from texture atlas.\n 	if(In.uv.y > 0.0){\n 		// Select a sprite, depending on time and flash id.\n 		float shift = floor(mod(15.0 * time, numberSprites)) + floor(rand(In.id * vec2(time,1.0)));\n 		vec2 globalUV = vec2(0.5 * mod(shift, 2.0), 0.25 * floor(shift/2.0));\n 		\n 		// Scale UV to fit in one sprite from atlas.\n 		vec2 localUV = In.uv * 0.5 + vec2(0.25,-0.25);\n 		localUV.y = min(-0.05,localUV.y); //Safety clamp on the upper side (or you could set clamp_t)\n 		\n 		// Read in black and white texture do determine opacity (mask).\n 		vec2 finalUV = globalUV + localUV;\n 		mask = texture(textureFlash,finalUV).r;\n 	}\n 	\n 	// Colored sprite.\n 	vec4 spriteColor = vec4(baseColor[cid], mask);\n 	\n 	// Circular halo effect.\n 	float haloAlpha = 1.0 - smoothstep(0.07,0.5,length(In.uv));\n 	vec4 haloColor = vec4(1.0,1.0,1.0, haloAlpha * 0.92);\n 	\n 	// Mix the sprite color and the halo effect.\n 	fragColor = mix(spriteColor, haloColor, haloColor.a);\n 	\n 	// Boost intensity.\n 	fragColor *= 1.1;\n 	\n }\n "},
{ "notes_vert", "#version 330\n layout(location = 0) in vec2 v;\n layout(location = 1) in vec4 id; //note id, start, duration, is minor\n layout(location = 2) in float channel; //note id, start, duration, is minor\n uniform float time;\n uniform float mainSpeed;\n uniform float minorsWidth = 1.0;\n uniform float keyboardHeight = 0.25;\n uniform int minNoteMajor;\n uniform float notesCount;\n out INTERFACE {\n 	vec2 uv;\n 	vec2 noteSize;\n 	float isMinor;\n 	float channel;\n } Out;\n void main(){\n 	\n 	float scalingFactor = id.w != 0.0 ? minorsWidth : 1.0;\n 	// Size of the note : width, height based on duration and current speed.\n 	Out.noteSize = vec2(0.9*2.0/notesCount * scalingFactor, id.z*mainSpeed);\n 	\n 	// Compute note shift.\n 	// Horizontal shift based on note id, width of keyboard, and if the note is minor or not.\n 	// Vertical shift based on note start time, current time, speed, and height of the note quad.\n 	//float a = (1.0/(notesCount-1.0)) * (2.0 - 2.0/notesCount);\n 	//float b = -1.0 + 1.0/notesCount;\n 	// This should be in -1.0, 1.0.\n 	// input: id.x is in [0 MAJOR_COUNT]\n 	// we want minNote to -1+1/c, maxNote to 1-1/c\n 	float a = 2.0;\n 	float b = -notesCount + 1.0 - 2.0 * float(minNoteMajor);\n 	float horizLoc = (id.x * a + b + id.w) / notesCount;\n 	float vertLoc = (Out.noteSize.y * 0.5 + (2.0 * keyboardHeight - 1.0)) + mainSpeed * (id.y - time);\n 	vec2 noteShift = vec2(horizLoc, vertLoc);\n 	\n 	// Scale uv.\n 	Out.uv = Out.noteSize * v;\n 	Out.isMinor = id.w;\n 	Out.channel = channel;\n 	// Output position.\n 	gl_Position = vec4(Out.noteSize * v + noteShift, 0.0 , 1.0) ;\n 	\n }\n "}, 
{ "notes_frag", "#version 330\n #define CHANNELS_COUNT 8\n in INTERFACE {\n 	vec2 uv;\n 	vec2 noteSize;\n 	float isMinor;\n 	float channel;\n } In;\n uniform vec3 baseColor[CHANNELS_COUNT];\n uniform vec3 minorColor[CHANNELS_COUNT];\n uniform vec2 inverseScreenSize;\n uniform float colorScale;\n uniform float keyboardHeight = 0.25;\n #define cornerRadius 0.01\n out vec4 fragColor;\n void main(){\n 	\n 	// If lower area of the screen, discard fragment as it should be hidden behind the keyboard.\n 	if(gl_FragCoord.y < keyboardHeight/inverseScreenSize.y){\n 		discard;\n 	}\n 	\n 	// Rounded corner (super-ellipse equation).\n 	float radiusPosition = pow(abs(In.uv.x/(0.5*In.noteSize.x)), In.noteSize.x/cornerRadius) + pow(abs(In.uv.y/(0.5*In.noteSize.y)), In.noteSize.y/cornerRadius);\n 	\n 	if(	radiusPosition > 1.0){\n 		discard;\n 	}\n 	\n 	// Fragment color.\n 	int cid = int(In.channel);\n 	fragColor.rgb = colorScale * mix(baseColor[cid], minorColor[cid], In.isMinor);\n 	\n 	if(	radiusPosition > 0.8){\n 		fragColor.rgb *= 1.05;\n 	}\n 	fragColor.a = 1.0;\n }\n "},
{ "particles_vert", "#version 330\n #define CHANNELS_COUNT 8\n layout(location = 0) in vec2 v;\n uniform float time;\n uniform float scale;\n uniform vec3 baseColor[CHANNELS_COUNT];\n uniform vec2 inverseScreenSize;\n uniform sampler2D textureParticles;\n uniform vec2 inverseTextureSize;\n uniform int globalId;\n uniform float duration;\n uniform int channel;\n uniform int texCount;\n uniform float colorScale;\n uniform float expansionFactor = 1.0;\n uniform float speedScaling = 0.2;\n uniform float keyboardHeight = 0.25;\n uniform int minNote;\n uniform float notesCount;\n const float shifts[128] = float[](\n 0,0.5,1,1.5,2,3,3.5,4,4.5,5,5.5,6,7,7.5,8,8.5,9,10,10.5,11,11.5,12,12.5,13,14,14.5,15,15.5,16,17,17.5,18,18.5,19,19.5,20,21,21.5,22,22.5,23,24,24.5,25,25.5,26,26.5,27,28,28.5,29,29.5,30,31,31.5,32,32.5,33,33.5,34,35,35.5,36,36.5,37,38,38.5,39,39.5,40,40.5,41,42,42.5,43,43.5,44,45,45.5,46,46.5,47,47.5,48,49,49.5,50,50.5,51,52,52.5,53,53.5,54,54.5,55,56,56.5,57,57.5,58,59,59.5,60,60.5,61,61.5,62,63,63.5,64,64.5,65,66,66.5,67,67.5,68,68.5,69,70,70.5,71,71.5,72,73,73.5,74\n );\n out INTERFACE {\n 	vec4 color;\n 	vec2 uv;\n 	float id;\n } Out;\n float rand(vec2 co){\n 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n }\n void main(){\n 	Out.id = float(gl_InstanceID % texCount);\n 	Out.uv = v + 0.5;\n 	// Fade color based on time.\n 	Out.color = vec4(colorScale * baseColor[channel], 1.0-time*time);\n 	\n 	float localTime = speedScaling * time * duration;\n 	float particlesCount = 1.0/inverseTextureSize.y;\n 	\n 	// Pick particle id at random.\n 	float particleId = float(gl_InstanceID) + floor(particlesCount * 10.0 * rand(vec2(globalId,globalId)));\n 	float textureId = mod(particleId,particlesCount);\n 	float particleShift = floor(particleId/particlesCount);\n 	\n 	// Particle uv, in pixels.\n 	vec2 particleUV = vec2(localTime / inverseTextureSize.x + 10.0 * particleShift, textureId);\n 	// UV in [0,1]\n 	particleUV = (particleUV+0.5)*vec2(1.0,-1.0)*inverseTextureSize;\n 	// Avoid wrapping.\n 	particleUV.x = clamp(particleUV.x,0.0,1.0);\n 	// We want to skip reading from the very beginning of the trajectories because they are identical.\n 	// particleUV.x = 0.95 * particleUV.x + 0.05;\n 	// Read corresponding trajectory to get particle current position.\n 	vec3 position = texture(textureParticles, particleUV).xyz;\n 	// Center position (from [0,1] to [-0.5,0.5] on x axis.\n 	position.x -= 0.5;\n 	\n 	// Compute shift, randomly disturb it.\n 	vec2 shift = 0.5*position.xy;\n 	float random = rand(vec2(particleId + float(globalId),time*0.000002+100.0*float(globalId)));\n 	shift += vec2(0.0,0.1*random);\n 	\n 	// Scale shift with time (expansion effect).\n 	shift = shift*time*expansionFactor;\n 	// and with altitude of the particle (ditto).\n 	shift.x *= max(0.5, pow(shift.y,0.3));\n 	\n 	// Horizontal shift is based on the note ID.\n 	float xshift = -1.0 + ((shifts[globalId] - shifts[int(minNote)]) * 2.0 + 1.0) / notesCount;\n 	//  Combine global shift (due to note id) and local shift (based on read position).\n 	vec2 globalShift = vec2(xshift, (2.0 * keyboardHeight - 1.0)-0.02);\n 	vec2 localShift = 0.003 * scale * v + shift * duration * vec2(1.0,0.5);\n 	vec2 screenScaling = vec2(1.0,inverseScreenSize.y/inverseScreenSize.x);\n 	vec2 finalPos = globalShift + screenScaling * localShift;\n 	\n 	// Discard particles that reached the end of their trajectories by putting them off-screen.\n 	finalPos = mix(vec2(-200.0),finalPos, position.z);\n 	// Output final particle position.\n 	gl_Position = vec4(finalPos,0.0,1.0);\n 	\n 	\n }\n "}, 
{ "particles_frag", "#version 330\n in INTERFACE {\n 	vec4 color;\n 	vec2 uv;\n 	float id;\n } In;\n uniform sampler2DArray lookParticles;\n out vec4 fragColor;\n void main(){\n 	float alpha = texture(lookParticles, vec3(In.uv, In.id)).r;\n 	fragColor = In.color;\n 	fragColor.a *= alpha;\n }\n "},
{ "particlesblur_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "particlesblur_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n uniform float attenuationFactor = 0.99;\n out vec4 fragColor;\n void main(){\n 	\n 	// We have to unroll the box blur loop manually.\n 	// 5x5 blur, using a sparse sample grid.\n 	vec4 color = texture(screenTexture, In.uv);\n 	\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(-2,-2));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(-2, 2));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(-1, 0));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2( 0,-1));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2( 0, 1));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2( 1, 0));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2( 2,-2));\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2( 2, 2));\n 	\n 	// Include decay for fade out.\n 	fragColor = mix(vec4(0.0), color/9.0, attenuationFactor);\n 	\n }\n "},
{ "screenquad_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "screenquad_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n out vec4 fragColor;\n void main(){\n 	\n 	fragColor = texture(screenTexture,In.uv);\n 	\n }\n "},
{ "keys_vert", "#version 330\n layout(location = 0) in vec2 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n uniform float keyboardHeight = 0.25;\n void main(){\n 	// Input are in -0.5,0.5\n 	// We directly output the position.\n 	// [-0.5, 0.5] to [-1, 2.0*keyboardHeight-1.0]\n 	float yShift = keyboardHeight * (2.0 * v.y + 1.0) - 1.0;\n 	gl_Position = vec4(v.x*2.0, yShift, 0.0, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy + 0.5;\n 	\n }\n "}, 
{ "keys_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n layout(std140) uniform ActiveNotes {\n 	ivec4 actives[32];\n };\n #define CHANNELS_COUNT 8\n #define MAJOR_COUNT 75\n uniform vec2 inverseScreenSize;\n uniform float minorsWidth = 1.0;\n uniform vec3 keysColor = vec3(0.0);\n uniform vec3 minorColor[CHANNELS_COUNT];\n uniform vec3 majorColor[CHANNELS_COUNT];\n uniform bool highlightKeys;\n uniform int minNoteMajor;\n uniform float notesCount;\n const bool isMinor[MAJOR_COUNT] = bool[](true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, true, true, false,  true, true, false, true, false);\n const int majorIds[MAJOR_COUNT] = int[](0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, 29, 31, 33, 35, 36, 38, 40, 41, 43, 45, 47, 48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84, 86, 88, 89, 91, 93, 95, 96, 98, 100, 101, 103, 105, 107, 108, 110, 112, 113, 115, 117, 119, 120, 122, 124, 125, 127);\n const int minorIds[MAJOR_COUNT] = int[](1, 3, 0, 6, 8, 10, 0, 13, 15, 0, 18, 20, 22, 0, 25, 27, 0, 30, 32, 34, 0, 37, 39, 0, 42, 44, 46, 0, 49, 51, 0, 54, 56, 58, 0, 61, 63, 0, 66, 68, 70, 0, 73, 75, 0, 78, 80, 82, 0, 85, 87, 0, 90, 92, 94, 0, 97, 99, 0, 102, 104, 106, 0, 109, 111, 0, 114, 116, 118, 0, 121, 123, 0, 126, 0);\n out vec4 fragColor;\n int isIdActive(int id){\n 	return actives[id/4][id%4];\n }\n void main(){\n 	// White keys: white\n 	// Black keys: keyColor\n 	// Lines between keys: keyColor\n 	// Active key: activeColor\n 	// White keys, and separators.\n 	float intensity = int(abs(fract(In.uv.x * notesCount)) >= 2.0 * notesCount * inverseScreenSize.x);\n 	\n 	// If the current major key is active, the majorColor is specific.\n 	int majorId = majorIds[clamp(int(In.uv.x * notesCount) + minNoteMajor, 0, 74)];\n 	int cidMajor = isIdActive(majorId);\n 	vec3 backColor = (highlightKeys && cidMajor >= 0) ? majorColor[cidMajor] : vec3(1.0);\n 	vec3 frontColor = keysColor;\n 	// Upper keyboard.\n 	if(In.uv.y > 0.4){\n 		int minorLocalId = min(int(floor(In.uv.x * notesCount + 0.5) + minNoteMajor) - 1, 74);\n 		// Handle black keys.\n 		// Hide keys that are on the edges.\n 		if(minorLocalId >= 0 && isMinor[minorLocalId] && In.uv.x > 0.5/notesCount && In.uv.x < 1.0 - 0.5/notesCount){\n 			// If the minor keys are not thinner, preserve a 1 px margin on each side.\n 			float marginSize = minorsWidth != 1.0 ? minorsWidth : 1.0 - (2.0 * notesCount * inverseScreenSize.x);\n 			intensity = step(marginSize, abs(fract(In.uv.x * notesCount + 0.5) * 2.0 - 1.0));\n 			int minorId = minorIds[minorLocalId];\n 			int cidMinor = isIdActive(minorId);\n 			if(highlightKeys && cidMinor >= 0){\n 				frontColor = minorColor[cidMinor];\n 			}\n 		}\n 	}\n 	\n 	fragColor.rgb = mix(frontColor, backColor, intensity);\n 	fragColor.a = 1.0;\n }\n "},
{ "backgroundtexture_vert", "#version 330\n layout(location = 0) in vec2 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n uniform bool behindKeyboard;\n uniform float keyboardHeight = 0.25;\n void main(){\n 	vec2 pos = v;\n 	if(!behindKeyboard){\n 		pos.y = (1.0-keyboardHeight) * pos.y + keyboardHeight;\n 	}\n 	// We directly output the position.\n 	gl_Position = vec4(pos, 0.0, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "backgroundtexture_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform float textureAlpha;\n uniform bool behindKeyboard;\n out vec4 fragColor;\n void main(){\n 	fragColor = texture(screenTexture, In.uv);\n 	fragColor.a *= textureAlpha;\n }\n "},
{ "pedal_vert", "#version 330\n layout(location = 0) in vec2 v;\n uniform vec2 shift;\n uniform vec2 scale;\n out INTERFACE {\n 	float id;\n } Out ;\n void main(){\n 	// Translate to put on top of the keyboard.\n 	gl_Position = vec4(v.xy * scale + shift, 0.5, 1.0);\n 	// Detect which pedal this vertex belong to.\n 	Out.id = gl_VertexID < 33 ? 0.0 : (gl_VertexID > 64 ? 2.0 : 1.0);\n 	\n }\n "}, 
{ "pedal_frag", "#version 330\n in INTERFACE {\n 	float id;\n } In ;\n uniform vec2 inverseScreenSize;\n uniform vec3 pedalColor;\n uniform ivec3 pedalFlags; // sostenuto, damper, soft\n uniform float pedalOpacity;\n uniform bool mergePedals;\n out vec4 fragColor;\n void main(){\n 	float vis = pedalOpacity;\n 	// When merging, only display the center pedal.\n 	if(mergePedals && (int(In.id) != 0)){\n 		discard;\n 	}\n 	// Else find if the current pedal (or any if merging) is active.\n 	for(int i = 0; i < 3; ++i){\n 		if((mergePedals || int(In.id) == i) && pedalFlags[i] > 0){\n 			vis = 1.0;\n 			break;\n 		}\n 	}\n 	\n 	fragColor = vec4(pedalColor, vis);\n }\n "},
{ "wave_vert", "#version 330\n layout(location = 0) in vec2 v;\n uniform float amplitude;\n uniform float time;\n uniform float shift;\n uniform float freqPos = 10.0;\n uniform float freqTime = 10.0;\n uniform float hash = 1.0;\n out INTERFACE {\n 	float grad;\n } Out ;\n void main(){\n 	// Translate to put on top of the keyboard.\n 	vec2 pos = vec2(1.0, 0.02) * v.xy;\n 	float waveShift = amplitude * sin(freqPos * v.x + freqTime * time + hash * 7.31);\n 	pos += vec2(0.0, waveShift + (-1.0 + 2.0 * shift));\n 	gl_Position = vec4(pos, 0.5, 1.0);\n 	Out.grad = v.y;\n }\n "}, 
{ "wave_frag", "#version 330\n in INTERFACE {\n 	float grad;\n } In ;\n uniform vec2 inverseScreenSize;\n uniform vec3 waveColor;\n out vec4 fragColor;\n void main(){\n 	float intensity = (1.0-abs(In.grad));\n 	fragColor = vec4((1.0+2.0*intensity)*waveColor, intensity);\n }\n "}
};
