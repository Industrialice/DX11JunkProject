#include "PreHeader.hpp"
#include "VectorFieldFileLoader.hpp"
#include <FileIO.hpp>
#include "Globals.hpp"
#include <FileMapping.hpp>
#include "RendererGlobals.hpp"

bln VectorFieldFileLoader::Load( const FilePath &path, VectorFieldInfo *info )
{
	if( info == nullptr )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> VectorFieldInfo info must point at something\n" );
		return false;
	}

	FileIO::fileError error;
	FileIO::CFile file( path, FileOpenMode::OpenExisting, FileProcMode::Read, FileCacheMode::LinearRead, &error );
	if( !file.IsOpened() )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> failed to open vector fields file\n" );
		return false;
	}

#pragma pack(1)
	struct FileHeader
	{
		char str[ 5 ];
		ui32 magic;
		ui32 version;
		ui32 cellsCounts[ 3 ];
		f64 bbMin[ 3 ];
		f64 bbMax[ 3 ];
	};
#pragma pack()

	ASSUME( alignof(FileHeader) == 1 );

	if( file.SizeGet() <= sizeof(FileHeader) )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> vector fields file's header is invalid( file is too small to contain the header )\n" );
		return false;
	}

	FileMapping::Mapping mapping( &file, 0, uiw_max, false );
	if( !mapping.IsOpened() )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> failed to create file mapping for a vector fields file\n" );
		return false;
	}

	FileHeader *header = (FileHeader *)mapping.CMemory();

	if( !_MemEquals( header->str, "IVFFF", 5 ) )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> vector fields file's header is invalid( file is too small to contain the header )\n" );
		return false;
	}

	if( header->magic != 0xFACEDBADu )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> vector fields file's magic is invalid, it is %h while %h is valid\n", header->magic, 0xFACEDBADu );
		return false;
	}

	if( header->version != 0 && header->version != 1 )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> vector fields file's magic version is unsupported, its version is %u while only 0 and 1 are currently supported\n", header->version );
		return false;
	}

	SENDLOG( CLogger::Tag::info, "VectorFieldFileLoader::Load -> vector fields file info:\n" );
	SENDLOG( CLogger::Tag::info, "cells count %u x %u x %u\n", header->cellsCounts[ 0 ], header->cellsCounts[ 1 ], header->cellsCounts[ 2 ] );
	SENDLOG( CLogger::Tag::info, "bbox min %f ; %f ; %f\n", header->bbMin[ 0 ], header->bbMin[ 1 ], header->bbMin[ 2 ] );
	SENDLOG( CLogger::Tag::info, "bbox max %f ; %f ; %f\n", header->bbMax[ 0 ], header->bbMax[ 1 ], header->bbMax[ 2 ] );
	
#pragma pack(1)
	const f64 *normals = (f64 *)((byte *)mapping.CMemory() + sizeof(FileHeader));
#pragma pack()

	D3D11_TEXTURE3D_DESC texDesc;
	texDesc.Width = header->cellsCounts[ 0 ];
	texDesc.Height = header->cellsCounts[ 1 ];
	texDesc.Depth = header->cellsCounts[ 2 ];
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA texData = {};

	UniquePtr < vec4 > converted;

	if( header->version == 0 )
	{
		SENDLOG( CLogger::Tag::warning, "VectorFieldFileLoader::Load -> reading f64 version of the vector field file, consider recompression info f32\n" );

		converted = new vec4[ header->cellsCounts[ 0 ] * header->cellsCounts[ 1 ] * header->cellsCounts[ 2 ] ];

		vec4 *convertedWriter = converted;

		for( ui32 d = 0; d < header->cellsCounts[ 2 ]; ++d )
		{
			for( ui32 h = 0; h < header->cellsCounts[ 1 ]; ++h )
			{
				for( ui32 w = 0; w < header->cellsCounts[ 0 ]; ++w )
				{
					vec3 vec( (f32)normals[ 0 ], (f32)normals[ 1 ], (f32)normals[ 2 ] );

					f32 len = LiceMath::Vec3Length( &vec );
					LiceMath::Vec3NormalizeInplace( &vec );

					*convertedWriter = vec4( vec, len );

					normals += 3;
					convertedWriter++;
				}
			}
		}

		texData.pSysMem = converted;
		texData.SysMemPitch = sizeof(vec4) * header->cellsCounts[ 0 ];
		texData.SysMemSlicePitch = sizeof(vec4) * header->cellsCounts[ 0 ] * header->cellsCounts[ 1 ];
	}
	else if( header->version == 1 )
	{
		SENDLOG( CLogger::Tag::error, "VectorFieldFileLoader::Load -> vector field files version 1 reading isn't implemented\n", header->version );

		texData.pSysMem = normals;
		SOFTBREAK;
		return false;
	}

	DXHRCHECK( RendererGlobals::i_Device->CreateTexture3D( &texDesc, &texData, &info->i_texture ) );

	info->width = header->cellsCounts[ 0 ];
	info->height = header->cellsCounts[ 1 ];
	info->depth = header->cellsCounts[ 2 ];

	info->bboxMin = vec3( (f32)header->bbMin[ 0 ], (f32)header->bbMin[ 1 ], (f32)header->bbMin[ 2 ] );
	info->bboxMax = vec3( (f32)header->bbMax[ 0 ], (f32)header->bbMax[ 1 ], (f32)header->bbMax[ 2 ] );

	SENDLOG( CLogger::Tag::info, "VectorFieldFileLoader::Load -> finished loading vector fields file\n" );

	return true;
}