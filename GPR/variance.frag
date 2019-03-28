#version 430 core

in float v_variance;
out vec4 color;


vec3 getHeatMapColor2(float value){


	int NUM_COLORS = 9;

	float color[9][3] = {{ 69.0 / 255, 4.0 / 255, 87.0 / 255 }, { 71.0 / 255, 42.0 / 255, 122.0 / 255 }, { 60.0 / 255, 80.0 / 255, 139.0 / 255 }, { 46.0 / 255, 111.0 / 255, 142.0 / 255 }, { 34.0 / 255, 140.0 / 255, 141.0 / 255 }, { 35.0 / 255, 169.0 / 255, 131.0 / 255 }, { 78.0 / 255, 195.0 / 255, 107.0 / 255 }, { 149.0 / 255, 216.0 / 255, 64.0 / 255 }, { 248.0 / 255, 230.0 / 255, 33.0 / 255 }};

	int idx1;
	int idx2;
	float fractBetween = 0;

		if (value <= 0){

			idx1 = idx2 = 0; 
 
		} else if (value >= 1)  { 

			idx1 = idx2 = NUM_COLORS - 1; 

		} else{
	
			value = value * (NUM_COLORS - 1);
			idx1 = int(floor(value));
			idx2 = idx1 + 1; 
			fractBetween = value - float(idx1);
		}

	return vec3((color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0], 
				(color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1], 
				(color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2]);

}

vec3 getHeatMapColor(float value){

	int NUM_COLORS = 3;

	float color[3][3] = {{ 0.0, 0.0, 1.0 },   
					{ 1.0, 1.0, 1.0 },   
					{ 1.0, 0.0, 0.0 }}; 

	int idx1;
	int idx2;
	float fractBetween = 0;

		if (value <= 0){

			idx1 = idx2 = 0; 
 
		} else if (value >= 1)  { 

			idx1 = idx2 = NUM_COLORS - 1; 

		} else{
	
			value = value * (NUM_COLORS - 1);
			idx1 = int(floor(value));
			idx2 = idx1 + 1; 
			fractBetween = value - float(idx1);
		}

	return vec3((color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0], 
				(color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1], 
				(color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2]);

}

void main(void){

	color = vec4(getHeatMapColor(v_variance ), 1.0);
	//float variance = (v_variance -0.44)*15;
	
	//color = vec4(getHeatMapColor(variance ), 1.0);
	
}
