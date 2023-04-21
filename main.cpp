//**************************************************************************************
//  Copyright (C) 2002 - 2023, Huamin Wang
//  All rights reserved.
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//     1. Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//     3. The names of its contributors may not be used to endorse or promote
//        products derived from this software without specific prior written
//        permission.
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//**************************************************************************************
// main.cpp
//**************************************************************************************
#include "GLSL_Utility.h"


class SBO_Array
{
	GLuint			sbo		= 0;
	unsigned int	size	= 0;

public:
	SBO_Array()
	{
		glGenBuffers(1, &sbo);
	}

	~SBO_Array()
	{
		glDeleteBuffers(1, &sbo);
	}

	void Allocate(unsigned int _size, bool is_static = false)
	{
		size = _size;

		//Allocate SBO buffer memory
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo);
		if (is_static)	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STATIC_DRAW);
		else			glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	}

	void Bind(unsigned int location)
	{
		//Bind the SBO to the program parameter at the specified location
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, sbo);
	}

	void Copy_In(const void* data)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STREAM_READ);
	}

	void Copy_Out(void* data)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
	}
};


int main(int argc, char *argv[])
{	
	//Initialize OpenGL by GLUT (or any other library)
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("Compute Shader Test");

	//Initialize GLEW and check GPU status
	GLenum err = glewInit();
	if (err != GLEW_OK)  printf(" Error initializing GLEW!\n");
	else printf("Initializing GLEW succeeded!\n");
	Check_GPU_Status();

	unsigned int vector_number	 = 1024;
	unsigned int work_group_size = 256;

	//Set up CPU data
	float* vector_A = new float[vector_number * 4] {};
	float* vector_B = new float[vector_number * 4] {};
	float* vector_R = new float[vector_number * 4] {};
	for (int i = 0; i < vector_number * 4; i++)
	{
		vector_A[i] = i % 3;
		vector_B[i] = i % 7;
	}

	//Set up GPU data (as SBO)
	SBO_Array sbo_A, sbo_B, sbo_R;
	sbo_A.Allocate(sizeof(float) * vector_number * 4);
	sbo_B.Allocate(sizeof(float) * vector_number * 4);
	sbo_R.Allocate(sizeof(float) * vector_number * 4);
	sbo_A.Copy_In(vector_A);
	sbo_B.Copy_In(vector_B);
	sbo_R.Copy_In(vector_R);

	//Build and run the program
	GLuint program = Setup_Compute("vec_add.comp");
	glUseProgram(program);
	sbo_A.Bind(1);
	sbo_B.Bind(2);
	sbo_R.Bind(3);
	glDispatchCompute(vector_number / work_group_size, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//Verify the output
	sbo_R.Copy_Out(vector_R);
	for (int i = 0; i < vector_number *4; i++)
	{
		printf("%d: %f + %f = %f\n", i, vector_A[i], vector_B[i], vector_R[i]);
	}		
	
	delete[] vector_A;
	delete[] vector_B;
	delete[] vector_R;
	
	return 0;
}