#pragma once

namespace StdLib { class FilePath; }

namespace VectorFieldFileLoader
{
	struct VectorFieldInfo
	{
		ui32 width, height, depth;
		vec3 bboxMin, bboxMax;
		ID3D11Texture3D *i_texture;
	};

	DX11_EXPORT bln Load( const class FilePath &path, VectorFieldInfo *info );
}