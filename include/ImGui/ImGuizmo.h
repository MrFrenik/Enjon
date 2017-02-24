// https://github.com/CedricGuillemet/ImGuizmo
// v 1.04 WIP
//
// The MIT License(MIT)
// 
// Copyright(c) 2016 Cedric Guillemet
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -------------------------------------------------------------------------------------------
// History : 
// 2016/09/11 Behind camera culling. Scaling Delta matrix not multiplied by source matrix scales. local/world rotation and translation fixed. Display message is incorrect (X: ... Y:...) in local mode.
// 2016/09/09 Hatched negative axis. Snapping. Documentation update.
// 2016/09/04 Axis switch and translation plan autohiding. Scale transform stability improved
// 2016/09/01 Mogwai changed to Manipulate. Draw debug cube. Fixed inverted scale. Mixing scale and translation/rotation gives bad results.
// 2016/08/31 First version
//
// -------------------------------------------------------------------------------------------
// Future (no order):
//
// - Multi view
// - display rotation/translation/scale infos in local/world space and not only local
// - finish local/world matrix application
// - OPERATION as bitmask
// 
// -------------------------------------------------------------------------------------------
// Example 
#if 0
void EditTransform(const Camera& camera, matrix_t& matrix)
{
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
	if (ImGui::IsKeyPressed(90))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix.m16, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation, 3);
	ImGui::InputFloat3("Rt", matrixRotation, 3);
	ImGui::InputFloat3("Sc", matrixScale, 3);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.m16);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}
	static bool useSnap(false);
	if (ImGui::IsKeyPressed(83))
		useSnap = !useSnap;
	ImGui::Checkbox("", &useSnap);
	ImGui::SameLine();
	vec_t snap;
	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		snap = config.mSnapTranslation;
		ImGui::InputFloat3("Snap", &snap.x);
		break;
	case ImGuizmo::ROTATE:
		snap = config.mSnapRotation;
		ImGui::InputFloat("Angle Snap", &snap.x);
		break;
	case ImGuizmo::SCALE:
		snap = config.mSnapScale;
		ImGui::InputFloat("Scale Snap", &snap.x);
		break;
	}

	ImGuizmo::Manipulate(camera.mView.m16, camera.mProjection.m16, mCurrentGizmoOperation, mCurrentGizmoMode, matrix.m16, NULL, useSnap ? &snap.x : NULL);
}
#endif
#pragma once

namespace ImGuizmo
{
	// call BeginFrame right after ImGui_XXXX_NewFrame();
	void BeginFrame();

	// return true if mouse cursor is over any gizmo control (axis, plan or screen component)
	bool IsOver();

	// return true if mouse IsOver or if the gizmo is in moving state
	bool IsUsing();

	// enable/disable the gizmo. Stay in the state until next call to Enable.
	// gizmo is rendered with gray half transparent color when disabled
	void Enable(bool enable);

	// helper functions for manualy editing translation/rotation/scale with an input float
	// translation, rotation and scale float points to 3 floats each
	// Angles are in degrees (more suitable for human editing)
	// example:
	// float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	// ImGuizmo::DecomposeMatrixToComponents(gizmoMatrix.m16, matrixTranslation, matrixRotation, matrixScale);
	// ImGui::InputFloat3("Tr", matrixTranslation, 3);
	// ImGui::InputFloat3("Rt", matrixRotation, 3);
	// ImGui::InputFloat3("Sc", matrixScale, 3);
	// ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, gizmoMatrix.m16);
	//
	// These functions have some numerical stability issues for now. Use with caution.
	void DecomposeMatrixToComponents(const float *matrix, float *translation, float *rotation, float *scale);
	void RecomposeMatrixFromComponents(const float *translation, const float *rotation, const float *scale, float *matrix);

	// Render a cube with face color corresponding to face normal. Usefull for debug/tests
	void DrawCube(const float *view, const float *projection, float *matrix);

	// call it when you want a gizmo
	// Needs view and projection matrices. 
	// matrix parameter is the source matrix (where will be gizmo be drawn) and might be transformed by the function. Return deltaMatrix is optional
	// translation is applied in world space
	enum OPERATION
	{
		TRANSLATE,
		ROTATE,
		SCALE
	};

	enum MODE
	{
		LOCAL,
		WORLD
	};

	void Manipulate(const float *view, const float *projection, OPERATION operation, MODE mode, float *matrix, float *deltaMatrix = 0, float *snap = 0, float *localBounds = NULL, float *boundsSnap = NULL);

	/*
	struct matrix_t
   {
   public:

      union
      {
         float m[4][4];
         float m16[16];
         struct
         {
            vec_t right, up, dir, position;
         } v;
		 vec_t component[4];
      };

      matrix_t(const matrix_t& other) { memcpy(&m16[0], &other.m16[0], sizeof(float) * 16); }
      matrix_t() {}

      operator float * () { return m16; }
      operator const float* () const { return m16; }
      void Translation(float _x, float _y, float _z) { this->Translation(makeVect(_x, _y, _z)); }

      void Translation(const vec_t& vt)
      {
         v.right.Set(1.f, 0.f, 0.f, 0.f);
         v.up.Set(0.f, 1.f, 0.f, 0.f);
         v.dir.Set(0.f, 0.f, 1.f, 0.f);
         v.position.Set(vt.x, vt.y, vt.z, 1.f);
      }

      void Scale(float _x, float _y, float _z)
      {
         v.right.Set(_x, 0.f, 0.f, 0.f);
         v.up.Set(0.f, _y, 0.f, 0.f);
         v.dir.Set(0.f, 0.f, _z, 0.f);
         v.position.Set(0.f, 0.f, 0.f, 1.f);
      }
      void Scale(const vec_t& s) { Scale(s.x, s.y, s.z); }

      matrix_t& operator *= (const matrix_t& mat)
      {
         matrix_t tmpMat;
         tmpMat = *this;
         tmpMat.Multiply(mat);
         *this = tmpMat;
         return *this;
      }
      matrix_t operator * (const matrix_t& mat) const
      {
         matrix_t matT;
         matT.Multiply(*this, mat);
         return matT;
      }

      void Multiply(const matrix_t &matrix)
      {
         matrix_t tmp;
         tmp = *this;

         FPU_MatrixF_x_MatrixF((float*)&tmp, (float*)&matrix, (float*)this);
      }

      void Multiply(const matrix_t &m1, const matrix_t &m2)
      {
         FPU_MatrixF_x_MatrixF((float*)&m1, (float*)&m2, (float*)this);
      }

      float GetDeterminant() const
      {
         return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[0][2] * m[1][0] * m[2][1] -
            m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
      }

      float Inverse(const matrix_t &srcMatrix, bool affine = false);
      void SetToIdentity() 
      {
         v.right.Set(1.f, 0.f, 0.f, 0.f);
         v.up.Set(0.f, 1.f, 0.f, 0.f);
         v.dir.Set(0.f, 0.f, 1.f, 0.f);
         v.position.Set(0.f, 0.f, 0.f, 1.f);
      }
      void Transpose()
      {
         matrix_t tmpm;
         for (int l = 0; l < 4; l++)
         {
            for (int c = 0; c < 4; c++)
            {
               tmpm.m[l][c] = m[c][l];
            }
         }
         (*this) = tmpm;
      }
      
      void RotationAxis(const vec_t & axis, float angle);

      void OrthoNormalize()
      {
         v.right.Normalize();
         v.up.Normalize();
         v.dir.Normalize();
      }
   };
   */
};