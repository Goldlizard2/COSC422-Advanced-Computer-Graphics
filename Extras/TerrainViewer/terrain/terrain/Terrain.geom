#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;
uniform float waterLevel;
uniform float snowLevel;
uniform int Tick;

out vec4 texWeights;
out float grassWeight;
out vec2 texCoord;
out float lightFactor;

float getWaterHeight(vec4 position)
{
	float m = 0.2;  // wave height
    float d = (waterLevel - position.y)/waterLevel;
    if(position.y < waterLevel){
		float m = position.y;  // wave height
	}
    float waterFrequency = 0.5; 
    float y = m * sin(waterFrequency * (d + float(Tick)*0.1));
    return y;
}


void main()
{
    float xmin = -45, xmax = +45, zmin = 0, zmax = -100;
	float dmax = 1.5;
	float waterWithRockLevel = 0.3;
	float rockWithGrassLevel = 1;
	float grassWithSnowLevel = 1;
	float grassLevel = 4;

	vec4 newPositions[3];

	for (int i = 0; i < gl_in.length(); i++) 
    {
        newPositions[i] = gl_in[i].gl_Position;
        if (newPositions[i].y < waterLevel){
            float waterHeight = getWaterHeight(newPositions[i]);
			newPositions[i].y = waterLevel + waterHeight;
		}
    }
	
	//face noraml of a triangle
	vec3 vector1 = newPositions[0].xyz - newPositions[2].xyz;
	vec3 vector2 = newPositions[1].xyz - newPositions[2].xyz;
	vec4 normal = vec4(normalize(cross(vector1, vector2)), 0);
    
    for (int i=0; i< gl_in.length(); i++)
	{
		vec4 oldPos = gl_in[i].gl_Position; //unmodified water position

        //pass texWeights to frag shader for selecting correct texture
		if (oldPos.y < waterLevel){               //water  
            texWeights = vec4(1.0, 0.0, 0.0, 0.0);
		}

		else if (getWaterHeight(newPositions[i]) > (waterLevel - waterWithRockLevel)){ //grass with snow
		    float waterWeight = (waterLevel - getWaterHeight(newPositions[i]))/waterWithRockLevel;
			float rockWeight = 1-waterWeight;
		    texWeights = vec4(waterWeight, rockWeight, 0.0, 0.0);
		}
	    
		else if (oldPos.y > (snowLevel - grassWithSnowLevel)){ //grass with snow
		    float grassWeight = (snowLevel - oldPos.y)/grassWithSnowLevel;
			float snowWeight = 1-grassWeight;
		    texWeights = vec4(0.0, 0.0, grassWeight, snowWeight);
		}
		else if (oldPos.y > (grassLevel - rockWithGrassLevel)){ //rock with grass
		    float rockWeight = (grassLevel - oldPos.y)/rockWithGrassLevel;
			float grassWeight = 1-rockWeight;
		    texWeights = vec4(0.0, rockWeight, grassWeight , 0.0);
		}
		else if (oldPos.y > snowLevel){
            texWeights = vec4(0.0, 0.0, 0.0, 1.0);    //snow
        }
		else if(oldPos.y > (grassLevel))
		{
			texWeights = vec4(0.0, 0.0, 1.0, 0.0);
		}
		else{
            texWeights = vec4(0.0, 1.0, 0.0, 0.0);  //rock
        }
		
		//lighting calculations
		vec4 lightPosn = vec4(-50, 50, 60, 1.0);

		//ambient
		float ambient = 0.2;
		
		//diffuse
		vec4 posnEye = newPositions[i];
		vec4 normalEye = normal;
		vec4 lgtVec = normalize(lightPosn - posnEye); 
		float diffuse = max(dot(lgtVec, normalEye), 0);   
		
		//specular for water
		float shininess = 100.0;
		vec4 white = vec4(1.0);
		vec4 viewVec = normalize(vec4(-posnEye.xyz, 0)); 		
		vec4 halfVec = normalize(lgtVec + viewVec); 
		float specular = max(dot(halfVec, normalEye), 0) * texWeights.x;
		
		//water depth variation
		float depth = newPositions[i].y - oldPos.y;
        float depthFactor = depth / dmax;

		//sum light, output to frag shader
		lightFactor = min(ambient + diffuse + specular - depthFactor, 1.0);
		
		texCoord.s = (newPositions[i].x - xmin) / (xmax - xmin);
		texCoord.t = (newPositions[i].z - zmin) / (zmax - zmin);

		gl_Position = mvpMatrix * newPositions[i];
		EmitVertex();	
	}
	EndPrimitive();
}