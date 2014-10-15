/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "mesh.h"
#include "debug.h"
#include "log.h"
#include <iostream>
#include <IAnimatedMesh.h>
#include <SAnimatedMesh.h>

// In Irrlicht 1.8 the signature of ITexture::lock was changed from
// (bool, u32) to (E_TEXTURE_LOCK_MODE, u32).
#if IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR <= 7
#define MY_ETLM_READ_ONLY true
#else
#define MY_ETLM_READ_ONLY video::ETLM_READ_ONLY
#endif

scene::IAnimatedMesh* createCubeMesh(v3f scale)
{
	video::SColor c(255,255,255,255);
	video::S3DVertex vertices[24] =
	{
		// Up
		video::S3DVertex(-0.5,+0.5,-0.5, 0,1,0, c, 0,1),
		video::S3DVertex(-0.5,+0.5,+0.5, 0,1,0, c, 0,0),
		video::S3DVertex(+0.5,+0.5,+0.5, 0,1,0, c, 1,0),
		video::S3DVertex(+0.5,+0.5,-0.5, 0,1,0, c, 1,1),
		// Down
		video::S3DVertex(-0.5,-0.5,-0.5, 0,-1,0, c, 0,0),
		video::S3DVertex(+0.5,-0.5,-0.5, 0,-1,0, c, 1,0),
		video::S3DVertex(+0.5,-0.5,+0.5, 0,-1,0, c, 1,1),
		video::S3DVertex(-0.5,-0.5,+0.5, 0,-1,0, c, 0,1),
		// Right
		video::S3DVertex(+0.5,-0.5,-0.5, 1,0,0, c, 0,1),
		video::S3DVertex(+0.5,+0.5,-0.5, 1,0,0, c, 0,0),
		video::S3DVertex(+0.5,+0.5,+0.5, 1,0,0, c, 1,0),
		video::S3DVertex(+0.5,-0.5,+0.5, 1,0,0, c, 1,1),
		// Left
		video::S3DVertex(-0.5,-0.5,-0.5, -1,0,0, c, 1,1),
		video::S3DVertex(-0.5,-0.5,+0.5, -1,0,0, c, 0,1),
		video::S3DVertex(-0.5,+0.5,+0.5, -1,0,0, c, 0,0),
		video::S3DVertex(-0.5,+0.5,-0.5, -1,0,0, c, 1,0),
		// Back
		video::S3DVertex(-0.5,-0.5,+0.5, 0,0,1, c, 1,1),
		video::S3DVertex(+0.5,-0.5,+0.5, 0,0,1, c, 0,1),
		video::S3DVertex(+0.5,+0.5,+0.5, 0,0,1, c, 0,0),
		video::S3DVertex(-0.5,+0.5,+0.5, 0,0,1, c, 1,0),
		// Front
		video::S3DVertex(-0.5,-0.5,-0.5, 0,0,-1, c, 0,1),
		video::S3DVertex(-0.5,+0.5,-0.5, 0,0,-1, c, 0,0),
		video::S3DVertex(+0.5,+0.5,-0.5, 0,0,-1, c, 1,0),
		video::S3DVertex(+0.5,-0.5,-0.5, 0,0,-1, c, 1,1),
	};

	u16 indices[6] = {0,1,2,2,3,0};

	scene::SMesh *mesh = new scene::SMesh();
	for (u32 i=0; i<6; ++i)
	{
		scene::IMeshBuffer *buf = new scene::SMeshBuffer();
		buf->append(vertices + 4 * i, 4, indices, 6);
		// Set default material
		buf->getMaterial().setFlag(video::EMF_LIGHTING, false);
		buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, false);
		buf->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
		// Add mesh buffer to mesh
		mesh->addMeshBuffer(buf);
		buf->drop();
	}

	scene::SAnimatedMesh *anim_mesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	scaleMesh(anim_mesh, scale);  // also recalculates bounding box
	return anim_mesh;
}

static scene::IAnimatedMesh* extrudeARGB(u32 twidth, u32 theight, u8 *data)
{
	const s32 argb_wstep = 4 * twidth;
	const s32 alpha_threshold = 1;

	scene::IMeshBuffer *buf = new scene::SMeshBuffer();
	video::SColor c(255,255,255,255);

	// Front and back
	{
		video::S3DVertex vertices[8] =
		{
			video::S3DVertex(-0.5,-0.5,-0.5, 0,0,-1, c, 0,1),
			video::S3DVertex(-0.5,+0.5,-0.5, 0,0,-1, c, 0,0),
			video::S3DVertex(+0.5,+0.5,-0.5, 0,0,-1, c, 1,0),
			video::S3DVertex(+0.5,-0.5,-0.5, 0,0,-1, c, 1,1),
			video::S3DVertex(+0.5,-0.5,+0.5, 0,0,+1, c, 1,1),
			video::S3DVertex(+0.5,+0.5,+0.5, 0,0,+1, c, 1,0),
			video::S3DVertex(-0.5,+0.5,+0.5, 0,0,+1, c, 0,0),
			video::S3DVertex(-0.5,-0.5,+0.5, 0,0,+1, c, 0,1),
		};
		u16 indices[12] = {0,1,2,2,3,0,4,5,6,6,7,4};
		buf->append(vertices, 8, indices, 12);
	}

	// "Interior"
	// (add faces where a solid pixel is next to a transparent one)
	u8 *solidity = new u8[(twidth+2) * (theight+2)];
	u32 wstep = twidth + 2;
	for (u32 y = 0; y < theight + 2; ++y)
	{
		u8 *scanline = solidity + y * wstep;
		if (y == 0 || y == theight + 1)
		{
			for (u32 x = 0; x < twidth + 2; ++x)
				scanline[x] = 0;
		}
		else
		{
			scanline[0] = 0;
			u8 *argb_scanline = data + (y - 1) * argb_wstep;
			for (u32 x = 0; x < twidth; ++x)
				scanline[x+1] = (argb_scanline[x*4+3] >= alpha_threshold);
			scanline[twidth + 1] = 0;
		}
	}

	// without this, there would be occasional "holes" in the mesh
	f32 eps = 0.01;

	for (u32 y = 0; y <= theight; ++y)
	{
		u8 *scanline = solidity + y * wstep + 1;
		for (u32 x = 0; x <= twidth; ++x)
		{
			if (scanline[x] && !scanline[x + wstep])
			{
				u32 xx = x + 1;
				while (scanline[xx] && !scanline[xx + wstep])
					++xx;
				f32 vx1 = (x - eps) / (f32) twidth - 0.5;
				f32 vx2 = (xx + eps) / (f32) twidth - 0.5;
				f32 vy = 0.5 - (y - eps) / (f32) theight;
				f32 tx1 = x / (f32) twidth;
				f32 tx2 = xx / (f32) twidth;
				f32 ty = (y - 0.5) / (f32) theight;
				video::S3DVertex vertices[8] =
				{
					video::S3DVertex(vx1,vy,-0.5, 0,-1,0, c, tx1,ty),
					video::S3DVertex(vx2,vy,-0.5, 0,-1,0, c, tx2,ty),
					video::S3DVertex(vx2,vy,+0.5, 0,-1,0, c, tx2,ty),
					video::S3DVertex(vx1,vy,+0.5, 0,-1,0, c, tx1,ty),
				};
				u16 indices[6] = {0,1,2,2,3,0};
				buf->append(vertices, 4, indices, 6);
				x = xx - 1;
			}
			if (!scanline[x] && scanline[x + wstep])
			{
				u32 xx = x + 1;
				while (!scanline[xx] && scanline[xx + wstep])
					++xx;
				f32 vx1 = (x - eps) / (f32) twidth - 0.5;
				f32 vx2 = (xx + eps) / (f32) twidth - 0.5;
				f32 vy = 0.5 - (y + eps) / (f32) theight;
				f32 tx1 = x / (f32) twidth;
				f32 tx2 = xx / (f32) twidth;
				f32 ty = (y + 0.5) / (f32) theight;
				video::S3DVertex vertices[8] =
				{
					video::S3DVertex(vx1,vy,-0.5, 0,1,0, c, tx1,ty),
					video::S3DVertex(vx1,vy,+0.5, 0,1,0, c, tx1,ty),
					video::S3DVertex(vx2,vy,+0.5, 0,1,0, c, tx2,ty),
					video::S3DVertex(vx2,vy,-0.5, 0,1,0, c, tx2,ty),
				};
				u16 indices[6] = {0,1,2,2,3,0};
				buf->append(vertices, 4, indices, 6);
				x = xx - 1;
			}
		}
	}

	for (u32 x = 0; x <= twidth; ++x)
	{
		u8 *scancol = solidity + x + wstep;
		for (u32 y = 0; y <= theight; ++y)
		{
			if (scancol[y * wstep] && !scancol[y * wstep + 1])
			{
				u32 yy = y + 1;
				while (scancol[yy * wstep] && !scancol[yy * wstep + 1])
					++yy;
				f32 vx = (x - eps) / (f32) twidth - 0.5;
				f32 vy1 = 0.5 - (y - eps) / (f32) theight;
				f32 vy2 = 0.5 - (yy + eps) / (f32) theight;
				f32 tx = (x - 0.5) / (f32) twidth;
				f32 ty1 = y / (f32) theight;
				f32 ty2 = yy / (f32) theight;
				video::S3DVertex vertices[8] =
				{
					video::S3DVertex(vx,vy1,-0.5, 1,0,0, c, tx,ty1),
					video::S3DVertex(vx,vy1,+0.5, 1,0,0, c, tx,ty1),
					video::S3DVertex(vx,vy2,+0.5, 1,0,0, c, tx,ty2),
					video::S3DVertex(vx,vy2,-0.5, 1,0,0, c, tx,ty2),
				};
				u16 indices[6] = {0,1,2,2,3,0};
				buf->append(vertices, 4, indices, 6);
				y = yy - 1;
			}
			if (!scancol[y * wstep] && scancol[y * wstep + 1])
			{
				u32 yy = y + 1;
				while (!scancol[yy * wstep] && scancol[yy * wstep + 1])
					++yy;
				f32 vx = (x + eps) / (f32) twidth - 0.5;
				f32 vy1 = 0.5 - (y - eps) / (f32) theight;
				f32 vy2 = 0.5 - (yy + eps) / (f32) theight;
				f32 tx = (x + 0.5) / (f32) twidth;
				f32 ty1 = y / (f32) theight;
				f32 ty2 = yy / (f32) theight;
				video::S3DVertex vertices[8] =
				{
					video::S3DVertex(vx,vy1,-0.5, -1,0,0, c, tx,ty1),
					video::S3DVertex(vx,vy2,-0.5, -1,0,0, c, tx,ty2),
					video::S3DVertex(vx,vy2,+0.5, -1,0,0, c, tx,ty2),
					video::S3DVertex(vx,vy1,+0.5, -1,0,0, c, tx,ty1),
				};
				u16 indices[6] = {0,1,2,2,3,0};
				buf->append(vertices, 4, indices, 6);
				y = yy - 1;
			}
		}
	}

	delete[] solidity;

	// Add to mesh
	scene::SMesh *mesh = new scene::SMesh();
	mesh->addMeshBuffer(buf);
	buf->drop();
	scene::SAnimatedMesh *anim_mesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	return anim_mesh;
}

scene::IAnimatedMesh* createExtrudedMesh(video::ITexture *texture,
		video::IVideoDriver *driver, v3f scale)
{
	scene::IAnimatedMesh *mesh = NULL;
	core::dimension2d<u32> size = texture->getOriginalSize();
	video::ECOLOR_FORMAT format = texture->getColorFormat();
	if (format == video::ECF_A8R8G8B8)
	{
		// Texture is in the correct color format, we can pass it
		// to extrudeARGB right away.
		void *data = texture->lock(MY_ETLM_READ_ONLY);
		if (data == NULL)
			return NULL;
		mesh = extrudeARGB(size.Width, size.Height, (u8*) data);
		texture->unlock();
	}
	else
	{
		video::IImage *img1 = driver->createImageFromData(format, size, texture->lock(MY_ETLM_READ_ONLY));
		if (img1 == NULL)
			return NULL;

		// img1 is in the texture's color format, convert to 8-bit ARGB
		video::IImage *img2 = driver->createImage(video::ECF_A8R8G8B8, size);
		if (img2 == NULL)
		{
			img1->drop();
			return NULL;
		}

		img1->copyTo(img2);
		img1->drop();
		mesh = extrudeARGB(size.Width, size.Height, (u8*) img2->lock());
		img2->unlock();
		img2->drop();
	}

	// Set default material
	mesh->getMeshBuffer(0)->getMaterial().setTexture(0, texture);
	mesh->getMeshBuffer(0)->getMaterial().setFlag(video::EMF_LIGHTING, false);
	mesh->getMeshBuffer(0)->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, false);
	mesh->getMeshBuffer(0)->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

	scaleMesh(mesh, scale);  // also recalculates bounding box
	return mesh;
}

void scaleMesh(scene::IMesh *mesh, v3f scale)
{
	if(mesh == NULL)
		return;

	core::aabbox3d<f32> bbox;
	bbox.reset(0,0,0);

	u16 mc = mesh->getMeshBufferCount();
	for(u16 j=0; j<mc; j++)
	{
		scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		u16 vc = buf->getVertexCount();
		for(u16 i=0; i<vc; i++)
		{
			vertices[i].Pos *= scale;
		}
		buf->recalculateBoundingBox();

		// calculate total bounding box
		if(j == 0)
			bbox = buf->getBoundingBox();
		else
			bbox.addInternalBox(buf->getBoundingBox());
	}
	mesh->setBoundingBox(bbox);
}

void translateMesh(scene::IMesh *mesh, v3f vec)
{
	if(mesh == NULL)
		return;

	core::aabbox3d<f32> bbox;
	bbox.reset(0,0,0);

	u16 mc = mesh->getMeshBufferCount();
	for(u16 j=0; j<mc; j++)
	{
		scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		u16 vc = buf->getVertexCount();
		for(u16 i=0; i<vc; i++)
		{
			vertices[i].Pos += vec;
		}
		buf->recalculateBoundingBox();

		// calculate total bounding box
		if(j == 0)
			bbox = buf->getBoundingBox();
		else
			bbox.addInternalBox(buf->getBoundingBox());
	}
	mesh->setBoundingBox(bbox);
}

void setMeshColor(scene::IMesh *mesh, const video::SColor &color)
{
	if(mesh == NULL)
		return;
	
	u16 mc = mesh->getMeshBufferCount();
	for(u16 j=0; j<mc; j++)
	{
		scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		u16 vc = buf->getVertexCount();
		for(u16 i=0; i<vc; i++)
		{
			vertices[i].Color = color;
		}
	}
}

void setMeshColorByNormalXYZ(scene::IMesh *mesh,
		const video::SColor &colorX,
		const video::SColor &colorY,
		const video::SColor &colorZ)
{
	if(mesh == NULL)
		return;
	
	u16 mc = mesh->getMeshBufferCount();
	for(u16 j=0; j<mc; j++)
	{
		scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		u16 vc = buf->getVertexCount();
		for(u16 i=0; i<vc; i++)
		{
			f32 x = fabs(vertices[i].Normal.X);
			f32 y = fabs(vertices[i].Normal.Y);
			f32 z = fabs(vertices[i].Normal.Z);
			if(x >= y && x >= z)
				vertices[i].Color = colorX;
			else if(y >= z)
				vertices[i].Color = colorY;
			else
				vertices[i].Color = colorZ;

		}
	}
}

void rotateMeshBy6dFacedir(scene::IMesh *mesh, int facedir)
{		
	int axisdir = facedir>>2;
	facedir &= 0x03;

	u16 mc = mesh->getMeshBufferCount();
	for(u16 j = 0; j < mc; j++)
	{
		scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		u16 vc = buf->getVertexCount();
		for(u16 i=0; i<vc; i++)
		{
			switch (axisdir)
			{
			case 0:
				if(facedir == 1)
					vertices[i].Pos.rotateXZBy(-90);
				else if(facedir == 2)
					vertices[i].Pos.rotateXZBy(180);
				else if(facedir == 3)
					vertices[i].Pos.rotateXZBy(90);
				break;
			case 1: // z+
				vertices[i].Pos.rotateYZBy(90);
				if(facedir == 1)
					vertices[i].Pos.rotateXYBy(90);
				else if(facedir == 2)
					vertices[i].Pos.rotateXYBy(180);
				else if(facedir == 3)
					vertices[i].Pos.rotateXYBy(-90);
				break;
			case 2: //z-
				vertices[i].Pos.rotateYZBy(-90);
				if(facedir == 1)
					vertices[i].Pos.rotateXYBy(-90);
				else if(facedir == 2)
					vertices[i].Pos.rotateXYBy(180);
				else if(facedir == 3)
					vertices[i].Pos.rotateXYBy(90);
				break;
			case 3:  //x+
				vertices[i].Pos.rotateXYBy(-90);
				if(facedir == 1)
					vertices[i].Pos.rotateYZBy(90);
				else if(facedir == 2)
					vertices[i].Pos.rotateYZBy(180);
				else if(facedir == 3)
					vertices[i].Pos.rotateYZBy(-90);
				break;
			case 4:  //x-
				vertices[i].Pos.rotateXYBy(90);
				if(facedir == 1)
					vertices[i].Pos.rotateYZBy(-90);
				else if(facedir == 2)
					vertices[i].Pos.rotateYZBy(180);
				else if(facedir == 3)
					vertices[i].Pos.rotateYZBy(90);
				break;
			case 5:
				vertices[i].Pos.rotateXYBy(-180);
				if(facedir == 1)
					vertices[i].Pos.rotateXZBy(90);
				else if(facedir == 2)
					vertices[i].Pos.rotateXZBy(180);
				else if(facedir == 3)
					vertices[i].Pos.rotateXZBy(-90);
				break;
			default:
				break;
			}
		}
	}
}

void recalculateBoundingBox(scene::IMesh *src_mesh)
{
	core::aabbox3d<f32> bbox;
	bbox.reset(0,0,0);
	for(u16 j = 0; j < src_mesh->getMeshBufferCount(); j++)
	{
		scene::IMeshBuffer *buf = src_mesh->getMeshBuffer(j);
		buf->recalculateBoundingBox();
		if(j == 0)
			bbox = buf->getBoundingBox();
		else
			bbox.addInternalBox(buf->getBoundingBox());
	}
	src_mesh->setBoundingBox(bbox);
}

scene::IMesh* cloneMesh(scene::IMesh *src_mesh)
{
	scene::SMesh* dst_mesh = new scene::SMesh();
	for(u16 j = 0; j < src_mesh->getMeshBufferCount(); j++)
	{
		scene::IMeshBuffer *buf = src_mesh->getMeshBuffer(j);
		video::S3DVertex *vertices = (video::S3DVertex*)buf->getVertices();
		u16 *indices = (u16*)buf->getIndices();
		scene::SMeshBuffer *temp_buf = new scene::SMeshBuffer();
		temp_buf->append(vertices, buf->getVertexCount(),
			indices, buf->getIndexCount());
		dst_mesh->addMeshBuffer(temp_buf);
		temp_buf->drop();
	}
	return dst_mesh;					
}

scene::IMesh* convertNodeboxNodeToMesh(ContentFeatures *f)
{
	scene::SMesh* dst_mesh = new scene::SMesh();
	for (u16 j = 0; j < 6; j++)
	{
		scene::IMeshBuffer *buf = new scene::SMeshBuffer();
		dst_mesh->addMeshBuffer(buf);
		buf->drop();
	}
	
	video::SColor c(255,255,255,255);	

	std::vector<aabb3f> boxes = f->node_box.fixed;
		
	for(std::vector<aabb3f>::iterator
			i = boxes.begin();
			i != boxes.end(); i++)
	{
		aabb3f box = *i;

		f32 temp;
		if (box.MinEdge.X > box.MaxEdge.X)
			{
				temp=box.MinEdge.X;
				box.MinEdge.X=box.MaxEdge.X;
				box.MaxEdge.X=temp;
			}
		if (box.MinEdge.Y > box.MaxEdge.Y)
			{
				temp=box.MinEdge.Y;
				box.MinEdge.Y=box.MaxEdge.Y;
				box.MaxEdge.Y=temp;
			}
		if (box.MinEdge.Z > box.MaxEdge.Z)
			{
				temp=box.MinEdge.Z;
				box.MinEdge.Z=box.MaxEdge.Z;
				box.MaxEdge.Z=temp;
			}
		// Compute texture coords
		f32 tx1 = (box.MinEdge.X/BS)+0.5;
		f32 ty1 = (box.MinEdge.Y/BS)+0.5;
		f32 tz1 = (box.MinEdge.Z/BS)+0.5;
		f32 tx2 = (box.MaxEdge.X/BS)+0.5;
		f32 ty2 = (box.MaxEdge.Y/BS)+0.5;
		f32 tz2 = (box.MaxEdge.Z/BS)+0.5;
		f32 txc[24] = {
			// up
			tx1, 1-tz2, tx2, 1-tz1,
			// down
			tx1, tz1, tx2, tz2,
			// right
			tz1, 1-ty2, tz2, 1-ty1,
			// left
			1-tz2, 1-ty2, 1-tz1, 1-ty1,
			// back
			1-tx2, 1-ty2, 1-tx1, 1-ty1,
			// front
			tx1, 1-ty2, tx2, 1-ty1,
		};
		v3f min = box.MinEdge;
		v3f max = box.MaxEdge;

		video::S3DVertex vertices[24] =
		{
			// up
			video::S3DVertex(min.X,max.Y,max.Z, 0,1,0, c, txc[0],txc[1]),
			video::S3DVertex(max.X,max.Y,max.Z, 0,1,0, c, txc[2],txc[1]),
			video::S3DVertex(max.X,max.Y,min.Z, 0,1,0, c, txc[2],txc[3]),
			video::S3DVertex(min.X,max.Y,min.Z, 0,1,0, c, txc[0],txc[3]),
			// down
			video::S3DVertex(min.X,min.Y,min.Z, 0,-1,0, c, txc[4],txc[5]),
			video::S3DVertex(max.X,min.Y,min.Z, 0,-1,0, c, txc[6],txc[5]),
			video::S3DVertex(max.X,min.Y,max.Z, 0,-1,0, c, txc[6],txc[7]),
			video::S3DVertex(min.X,min.Y,max.Z, 0,-1,0, c, txc[4],txc[7]),
			// right
			video::S3DVertex(max.X,max.Y,min.Z, 1,0,0, c, txc[ 8],txc[9]),
			video::S3DVertex(max.X,max.Y,max.Z, 1,0,0, c, txc[10],txc[9]),
			video::S3DVertex(max.X,min.Y,max.Z, 1,0,0, c, txc[10],txc[11]),
			video::S3DVertex(max.X,min.Y,min.Z, 1,0,0, c, txc[ 8],txc[11]),
			// left
			video::S3DVertex(min.X,max.Y,max.Z, -1,0,0, c, txc[12],txc[13]),
			video::S3DVertex(min.X,max.Y,min.Z, -1,0,0, c, txc[14],txc[13]),
			video::S3DVertex(min.X,min.Y,min.Z, -1,0,0, c, txc[14],txc[15]),
			video::S3DVertex(min.X,min.Y,max.Z, -1,0,0, c, txc[12],txc[15]),
			// back
			video::S3DVertex(max.X,max.Y,max.Z, 0,0,1, c, txc[16],txc[17]),
			video::S3DVertex(min.X,max.Y,max.Z, 0,0,1, c, txc[18],txc[17]),
			video::S3DVertex(min.X,min.Y,max.Z, 0,0,1, c, txc[18],txc[19]),
			video::S3DVertex(max.X,min.Y,max.Z, 0,0,1, c, txc[16],txc[19]),
			// front
			video::S3DVertex(min.X,max.Y,min.Z, 0,0,-1, c, txc[20],txc[21]),
			video::S3DVertex(max.X,max.Y,min.Z, 0,0,-1, c, txc[22],txc[21]),
			video::S3DVertex(max.X,min.Y,min.Z, 0,0,-1, c, txc[22],txc[23]),
			video::S3DVertex(min.X,min.Y,min.Z, 0,0,-1, c, txc[20],txc[23]),
		};

		u16 indices[] = {0,1,2,2,3,0};

		for(u16 j = 0; j < 24; j += 4)
		{
			scene::IMeshBuffer *buf = dst_mesh->getMeshBuffer(j / 4);
			buf->append(vertices + j, 4, indices, 6);
		}
	}
	return dst_mesh;					
}
