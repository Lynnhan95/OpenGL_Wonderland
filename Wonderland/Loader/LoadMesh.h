#ifndef __LOADMESH_H__
#define __LOADMESH_H__


#include <string>
#include <vector>
#include <glad/glad.h> // Switched to GLAD
#include <assimp/Scene.h>

struct SubmeshData
{
   unsigned int mNumIndices;
   unsigned int mBaseIndex;
   unsigned int mBaseVertex;

   SubmeshData() : mNumIndices(0), mBaseIndex(0), mBaseVertex(0) {} // Adjusted
   void DrawSubmesh();
   void DrawSubmeshInstanced(unsigned int count);
};

struct MeshData
{
   unsigned int mVao;
   unsigned int mVboVerts;
   unsigned int mVboNormals;
   unsigned int mVboTexCoords;
   unsigned int mIndexBuffer;
   float mScaleFactor; //TODO replace with bounding box

   const aiScene* mScene;
   aiVector3D mBbMin, mBbMax;

   std::vector<SubmeshData> mSubmesh;
   std::string mFilename;

   MeshData() : mVao(-1), mVboVerts(-1), mVboNormals(-1), mVboTexCoords(-1), mIndexBuffer(-1), mScaleFactor(0.0f), mScene(NULL) {} // Adjusted

   void DrawMesh();
   void DrawInstanced(unsigned int count);
};



MeshData LoadMesh(const std::string& pFile);


#endif