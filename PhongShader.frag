#version 450

uniform mat4 MODEL;

uniform vec3 cameraPosition;
uniform vec3 materialSpecularColor;

uniform float materialShininess;

uniform sampler2D myTextureSampler;
uniform sampler2D myNormalSampler;

uniform int textured;
uniform float gama;

in vec4 fragColor;
in vec4 fragPosition;
in vec3 fragNormale;
in vec2 fragUV;

out vec4 finalColor;

uniform struct Light {
	vec3 position;
	vec3 intensities;
	float ambientCoefficient;
	float attenuation;
} light;

void main() {
	
	vec4 TextureColor = texture(myTextureSampler, fragUV);
	vec4 TextureNorm = texture(myNormalSampler, fragUV);
	vec3 TextureNormale = 2.0 * abs(normalize(TextureNorm.xyz)) - 1.0;
	vec4 normal = vec4(normalize(mat3(MODEL)*(fragNormale+TextureNormale)), 1.0);
	if (textured==0) normal = vec4(normalize(mat3(MODEL)*fragNormale), 1.0);
	
	vec4 ambient = light.ambientCoefficient * vec4(light.intensities, 1.0) * TextureColor;
	if (textured==0) ambient = light.ambientCoefficient * vec4(light.intensities, 1.0) * fragColor;
	
	vec4 position = MODEL * fragPosition;
	vec4 lightPosition = vec4(light.position, 1.0);
	vec4 vecLight = normalize(lightPosition - position);
	
	float diffuseCoeff = max(0.0, dot(normal, vecLight));
	vec4 diffuse = diffuseCoeff * TextureColor * vec4(light.intensities, 1.0);
	if (textured==0) diffuse = diffuseCoeff * fragColor * vec4(light.intensities, 1.0);
	
	vec4 camPos = vec4(cameraPosition, 1.0);
	vec4 surfaceToCamera = normalize(camPos - position);
	vec3 reflectLight = reflect(-vecLight.xyz, normal.xyz);
	
	float specularCoeff = pow( max(0.0, dot(surfaceToCamera.xyz, reflectLight)), materialShininess);
	if (diffuseCoeff<=0.0) specularCoeff = 0.0;
	vec4 specular = vec4(specularCoeff * materialSpecularColor * light.intensities, 1.0);
	
	float distanceToLight = length(lightPosition - position);
	float attenuation = 1.0 / (1.0 + light.attenuation * pow(distanceToLight, 2));
	
	vec4 linearColor = ambient + attenuation * (diffuse + specular);
	
	finalColor = vec4(pow(linearColor.x, gama), pow(linearColor.y, gama), pow(linearColor.z, gama), 1.0);
	
}
