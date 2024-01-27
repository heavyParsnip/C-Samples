#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// My code -----------------------
	
	//Half of the height value for quick reference to help with centering the mesh
	float fHeightHalf = a_fHeight * 0.5f;

	//Calculate how much to rotate at each increment based on the number of subdivisions
	float fIncrement = 360.0f / a_nSubdivisions;

	//Track the current rotation, as well as the next iteration's rotation
	float fRotAngle = 0.0f;
	float fNextRotAngle = 0.0f;

	//C
	//| \
	//A--B

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Set current rotation
		fRotAngle = fIncrement * i;
		fNextRotAngle = fIncrement * (i + 1);

		//Get new rotation values in radians
		float fRotRadians = fRotAngle * ((float)PI / 180.0f);
		float fNextRotRadians = fNextRotAngle * ((float)PI / 180.0f);

		//Generate new points for this base face
		vector3 p0(cos(fRotRadians) * a_fRadius, -fHeightHalf, sin(fRotRadians)* a_fRadius);		  //A
		vector3 p1(cos(fNextRotRadians) * a_fRadius, -fHeightHalf, sin(fNextRotRadians) * a_fRadius); //B
		vector3 p2(0, -fHeightHalf, 0);																  //C, center of the base

		//Generate new points for this vertical face
		vector3 p3(cos(fRotRadians) * a_fRadius, -fHeightHalf, sin(fRotRadians) * a_fRadius);		  //A
		vector3 p4(cos(fNextRotRadians) * a_fRadius, -fHeightHalf, sin(fNextRotRadians) * a_fRadius); //B
		vector3 p5(0, fHeightHalf, 0);																  //C, apex of the cone


		//Build tris
		AddTri(p0, p1, p2);
		AddTri(p5, p4, p3);
	}
	// -------------------------------
	
	
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// My code -----------------------

	//Half of the height value for quick reference to help with centering the mesh
	float fHeightHalf = a_fHeight * 0.5f;

	//Calculate how much to rotate at each increment based on the number of subdivisions
	float fIncrement = 360.0f / a_nSubdivisions;

	//Track the current rotation, as well as the next iteration's rotation
	float fRotAngle = 0.0f;
	float fNextRotAngle = 0.0f;

	//C
	//| \
	//A--B

	//C--D
	//|  |
	//A--B
	//Draws A->B->C, then C->B->D

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Set current rotation
		fRotAngle = fIncrement * i;
		fNextRotAngle = fIncrement * (i + 1);

		//Get new rotation values in radians
		float fRotRadians = fRotAngle * ((float)PI / 180.0f);
		float fNextRotRadians = fNextRotAngle * ((float)PI / 180.0f);

		//Generate new points for this BOTTOM base face
		vector3 p0(cos(fRotRadians) * a_fRadius, -fHeightHalf, sin(fRotRadians) * a_fRadius);		  //A
		vector3 p1(cos(fNextRotRadians) * a_fRadius, -fHeightHalf, sin(fNextRotRadians) * a_fRadius); //B
		vector3 p2(0, -fHeightHalf, 0);																  //C, center of the BOTTOM base

		//Generate new points for this TOP base face
		vector3 p3(cos(fRotRadians) * a_fRadius, fHeightHalf, sin(fRotRadians) * a_fRadius);		  //A
		vector3 p4(cos(fNextRotRadians) * a_fRadius, fHeightHalf, sin(fNextRotRadians) * a_fRadius);  //B
		vector3 p5(0, fHeightHalf, 0);																  //C, apex of the TOP base


		//Build tris
		AddTri(p0, p1, p2); //Bottom base
		AddTri(p5, p4, p3); //Top base

		//Build quads, reusing points for the sides
		AddQuad(p3, p4, p0, p1); //Sides
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// My code -----------------------

	//Half of the height value for quick reference to help with centering the mesh
	float fHeightHalf = a_fHeight * 0.5f;

	//Calculate how much to rotate at each increment based on the number of subdivisions
	float fIncrement = 360.0f / a_nSubdivisions;

	//Track the current rotation, as well as the next iteration's rotation
	float fRotAngle = 0.0f;
	float fNextRotAngle = 0.0f;

	//C--D
	//|  |
	//A--B
	//Draws A->B->C, then C->B->D

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Set current rotation
		fRotAngle = fIncrement * i;
		fNextRotAngle = fIncrement * (i + 1);

		//Get new rotation values in radians
		float fRotRadians = fRotAngle * ((float)PI / 180.0f);
		float fNextRotRadians = fNextRotAngle * ((float)PI / 180.0f);

		//Generate new points for this BOTTOM base face
		vector3 p0(cos(fRotRadians) * a_fOuterRadius, -fHeightHalf, sin(fRotRadians) * a_fOuterRadius);			//A
		vector3 p1(cos(fNextRotRadians) * a_fOuterRadius, -fHeightHalf, sin(fNextRotRadians) * a_fOuterRadius); //B
		vector3 p2(cos(fRotRadians) * a_fInnerRadius, -fHeightHalf, sin(fRotRadians) * a_fInnerRadius);			//C
		vector3 p3(cos(fNextRotRadians) * a_fInnerRadius, -fHeightHalf, sin(fNextRotRadians) * a_fInnerRadius); //D

		//Generate new points for this TOP base face
		vector3 p4(cos(fRotRadians) * a_fOuterRadius, fHeightHalf, sin(fRotRadians) * a_fOuterRadius);			//A
		vector3 p5(cos(fNextRotRadians) * a_fOuterRadius, fHeightHalf, sin(fNextRotRadians) * a_fOuterRadius); //B
		vector3 p6(cos(fRotRadians) * a_fInnerRadius, fHeightHalf, sin(fRotRadians) * a_fInnerRadius);			//C
		vector3 p7(cos(fNextRotRadians) * a_fInnerRadius, fHeightHalf, sin(fNextRotRadians) * a_fInnerRadius); //D												  //C


		//Build quads, reusing points for the sides
		AddQuad(p0, p1, p2, p3); //Bottom base
		AddQuad(p6, p7, p4, p5); //Top base
		AddQuad(p4, p5, p0, p1); //Outer sides
		AddQuad(p2, p3, p6, p7); //Inner sides

	}
	// -------------------------------

	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// My code -----------------------
	//Quick reference for half of the radii, for scaling
	float fInnerRadiusHalf = a_fInnerRadius * 0.5f;
	float fOuterRadiusHalf = a_fOuterRadius * 0.5f;

	//Calculate how much to rotate at each increment based on the number of subdivisions
	float fIncrement = 360.0f / a_nSubdivisionsA;
	float fPhi = 360.0f / a_nSubdivisionsB;

	//Track the current rotation, as well as the next iteration's rotation
	float fRotAngle = 0.0f;
	float fNextRotAngle = 0.0f;

	float fPhiAngle = 0.0f;
	float fNextPhiAngle = 0.0f;

	//C--D
	//|  |
	//A--B
	//Draws A->B->C, then C->B->D

	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		

		//Set current theta rotation
		fRotAngle = fIncrement * i;
		fNextRotAngle = fIncrement * (i + 1);

		//Get new theta rotation values in radians
		float fRotRadians = fRotAngle * ((float)PI / 180.0f);
		float fNextRotRadians = fNextRotAngle * ((float)PI / 180.0f);

		for (int j = 0; j < a_nSubdivisionsB; j++)
		{
			//Set current phi rotation
			fPhiAngle = fPhi * j;
			fNextPhiAngle = fPhi * (j + 1);

			//Get new phi rotation values in radians
			float fPhiRadians = fPhiAngle * ((float)PI / 180.0f);
			float fNextPhiRadians = fNextPhiAngle * ((float)PI / 180.0f);

			//Parametric equations
			//X = cos(Theta) * (a*cos(Phi) + c)
			//Y = sin(Theta) * (a*cos(Phi) + c)
			//Z = a * sin(Phi)

			vector3 p0(cos(fRotRadians) * (fInnerRadiusHalf * cos(fPhiRadians) + a_fOuterRadius), sin(fRotRadians) * (fInnerRadiusHalf * cos(fPhiRadians) + a_fOuterRadius), fInnerRadiusHalf * sin(fPhiRadians));
			vector3 p1(cos(fRotRadians) * (fInnerRadiusHalf * cos(fNextPhiRadians) + a_fOuterRadius), sin(fRotRadians) * (fInnerRadiusHalf * cos(fNextPhiRadians) + a_fOuterRadius), fInnerRadiusHalf * sin(fNextPhiRadians));
			vector3 p2(cos(fNextRotRadians) * (fInnerRadiusHalf * cos(fPhiRadians) + a_fOuterRadius), sin(fNextRotRadians) * (fInnerRadiusHalf * cos(fPhiRadians) + a_fOuterRadius), fInnerRadiusHalf * sin(fPhiRadians));
			vector3 p3(cos(fNextRotRadians) * (fInnerRadiusHalf * cos(fNextPhiRadians) + a_fOuterRadius), sin(fNextRotRadians) * (fInnerRadiusHalf * cos(fNextPhiRadians) + a_fOuterRadius), fInnerRadiusHalf * sin(fNextPhiRadians));

			//Build quads
			AddQuad(p2, p3, p0, p1);
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// My code -----------------------
	//NOTE: I used the sphere primitive in Maya as a reference, so that's essentially the behavior I'm trying to imitate here

	//Calculate how much to rotate at each increment based on the number of subdivisions
	float fIncrement = 360.0f / a_nSubdivisions;
	float fPhi = 180.0f / a_nSubdivisions;

	//Track the current rotation, as well as the next iteration's rotation
	float fRotAngle = 0.0f;
	float fNextRotAngle = 0.0f;

	float fPhiAngle = 0.0f;
	float fNextPhiAngle = 0.0f;

	//C
	//| \
	//A--B

	//C--D
	//|  |
	//A--B
	//Draws A->B->C, then C->B->D

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//Set current theta rotation
		fRotAngle = fIncrement * i;
		fNextRotAngle = fIncrement * (i + 1);

		//Get new theta rotation values in radians
		float fRotRadians = fRotAngle * ((float)PI / 180.0f);
		float fNextRotRadians = fNextRotAngle * ((float)PI / 180.0f);

		for (int j = 0; j < a_nSubdivisions; j++)
		{
			//Set current phi rotation
			fPhiAngle = fPhi * j;
			fNextPhiAngle = fPhi * (j + 1);

			//Get new phi rotation values in radians
			float fPhiRadians = fPhiAngle * ((float)PI / 180.0f);
			float fNextPhiRadians = fNextPhiAngle * ((float)PI / 180.0f);

			//Parametric equations
			//X = R * sin(Phi) * cos(Theta)
			//Y = R * sin(Phi) * sin(Theta)
			//Z = R * cos(Phi)

			vector3 p0(a_fRadius * sin(fPhiRadians) * cos(fRotRadians), a_fRadius * sin(fPhiRadians) * sin(fRotRadians), a_fRadius * cos(fPhiRadians));
			vector3 p1(a_fRadius * sin(fNextPhiRadians)*cos(fRotRadians), a_fRadius * sin(fNextPhiRadians)*sin(fRotRadians), a_fRadius * cos(fNextPhiRadians));
			vector3 p2(a_fRadius * sin(fPhiRadians) * cos(fNextRotRadians), a_fRadius * sin(fPhiRadians) * sin(fNextRotRadians), a_fRadius * cos(fPhiRadians));
			vector3 p3(a_fRadius * sin(fNextPhiRadians) * cos(fNextRotRadians), a_fRadius * sin(fNextPhiRadians) * sin(fNextRotRadians), a_fRadius * cos(fNextPhiRadians));

			//Build quads
			AddQuad(p0, p1, p2, p3);
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}