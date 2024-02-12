//#####################################################################
// Main
// CS3451 Computer Graphics Starter Code
// Contact: Bo Zhu (bo.zhu@gatech.edu)
//#####################################################################
#include <iostream>
#include <random>
#include <unordered_set>

#include "OpenGLMesh.h"
#include "OpenGLCommon.h"
#include "OpenGLWindow.h"
#include "OpenGLViewer.h"
#include "TinyObjLoader.h"
#include "LoopSubdivision.h"

#ifndef __Main_cpp__
#define __Main_cpp__

class MeshDriver : public OpenGLViewer
{
	OpenGLTriangleMesh* opengl_tri_mesh=nullptr;						////mesh
	TriangleMesh<3>* tri_mesh=nullptr;
	OpenGLSegmentMesh* opengl_normals=nullptr;							////normals
	OpenGLSegmentMesh* opengl_edges=nullptr;							////edges

	bool use_obj_mesh=false;											////flag for use obj, set it to be true if you want to load an obj mesh
	std::string obj_mesh_name="Scaled_Capybara.obj";								////obj file name
	////There are two other obj files provided, cap.obj and jetFighter.obj.
	////They are both exported from resource library of Autodesk Maya 2020

public:
	virtual void Initialize()
	{
		OpenGLViewer::Initialize();
	}

	void Load_Mesh()
	{
		if(use_obj_mesh){
			std::vector<std::shared_ptr<TriangleMesh<3> > > meshes;
			Obj::Read_From_Obj_File(obj_mesh_name,meshes);
			*tri_mesh=*meshes[0];
			std::cout<<"load tri_mesh, #vtx: "<<tri_mesh->Vertices().size()<<", #ele: "<<tri_mesh->Elements().size()<<std::endl;		
		}
		else{
			Initialize_Icosahedron_Mesh(.5,tri_mesh);
		}	
	}

	virtual void Initialize_Data()
	{
		////initialize tri mesh
		opengl_tri_mesh=Add_Interactive_Object<OpenGLTriangleMesh>();
		tri_mesh=&opengl_tri_mesh->mesh;

		Load_Mesh();

		opengl_edges=Add_Interactive_Object<OpenGLSegmentMesh>();
		Set_Polygon_Mode(opengl_edges,PolygonMode::Fill);
		Set_Shading_Mode(opengl_edges,ShadingMode::None);
		Update_OpenGL_Seg_Mesh();

		Set_Polygon_Mode(opengl_tri_mesh,PolygonMode::Fill);
		Set_Shading_Mode(opengl_tri_mesh,ShadingMode::None);
		Set_Color(opengl_tri_mesh,OpenGLColor(.3f,.3f,.3f,1.f));
		opengl_tri_mesh->Set_Data_Refreshed();
		opengl_tri_mesh->Initialize();
	}

	////synchronize data to visualization
	void Sync_Simulation_And_Visualization_Data()
	{
		opengl_tri_mesh->Set_Data_Refreshed();
		opengl_tri_mesh->Initialize();
		Update_OpenGL_Seg_Mesh();
	}

	////update simulation and visualization for each time step
	virtual void Toggle_Next_Frame()
	{
		Sync_Simulation_And_Visualization_Data();
		OpenGLViewer::Toggle_Next_Frame();
	}

	virtual void Run()
	{
		OpenGLViewer::Run();
	}

	////Keyboard interaction
	virtual void Initialize_Common_Callback_Keys()
	{
		OpenGLViewer::Initialize_Common_Callback_Keys();
		Bind_Callback_Key('s',&Keyboard_Event_Subdivision_Func,"Loop subdivision");
		Bind_Callback_Key('r',&Keyboard_Event_Restore_Func,"Restore");
	}

	virtual void Keyboard_Event_Subdivision()
	{
		std::cout<<"Loop subdivision"<<std::endl;
		LoopSubdivision(*tri_mesh);
		Sync_Simulation_And_Visualization_Data();
		OpenGLViewer::Toggle_Next_Frame();
	}
	Define_Function_Object(MeshDriver,Keyboard_Event_Subdivision);

	virtual void Keyboard_Event_Restore()
	{
		frame=0;
		Load_Mesh();
		Sync_Simulation_And_Visualization_Data();
		OpenGLViewer::Toggle_Next_Frame();		
	}
	Define_Function_Object(MeshDriver,Keyboard_Event_Restore);

protected:
	void Get_Triangle_Mesh_Edges(const TriangleMesh<3>& triangle_mesh,std::unordered_set<Vector2i>& hashset)
	{
		for(int i=0;i<triangle_mesh.elements.size();i++){
			const Vector3i& tri=triangle_mesh.elements[i];
			for(int j=0;j<3;j++){
				Vector2i edge(tri[j],tri[(j+1)%3]);
				if(edge[0]>edge[1]){int tmp=edge[0];edge[0]=edge[1];edge[1]=tmp;}
				hashset.insert(edge);}}
	}

	void Update_OpenGL_Seg_Mesh()
	{
		auto seg_mesh=&opengl_edges->mesh;
		seg_mesh->elements.clear();
		std::unordered_set<Vector2i> edge_hashset;
		Get_Triangle_Mesh_Edges(*tri_mesh,edge_hashset);
		(*seg_mesh->vertices)=(*tri_mesh->vertices);
		for(auto& e:edge_hashset)
			seg_mesh->elements.push_back(e);
		opengl_edges->Set_Data_Refreshed();
		opengl_edges->Initialize();
		Set_Line_Width(opengl_edges,3.f);
		Set_Shading_Mode(opengl_edges,ShadingMode::None);
		Set_Color(opengl_edges,OpenGLColor(.1f,1.f,.1f,1.f));
	}
};

int main(int argc,char* argv[])
{
	MeshDriver driver;
	driver.Initialize();
	driver.Run();	
}

#endif