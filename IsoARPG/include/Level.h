#ifndef LEVEL_H
#define LEVEL_H

#include <vector>

//THIS ISN'T HOW THIS SHOULD BE DONE :: TODO:: SPRITESHEET WITH ALL LEVEL TILES INFORMATION INSTEAD
#include <Graphics/GLTexture.h>
#include <Graphics/SpriteBatch.h>
#include <Graphics/SpriteSheet.h>
#include <Math/Maths.h>
#include <Math/Random.h>

struct Tile
{
	Tile();
	//Tile(Enjon::Graphics::GLTexture Texture, Enjon::Math::Vec2 Pos, Enjon::Math::Vec2 Dims, unsigned int Index)
	//	: texture(Texture), pos(Pos), dims(Dims), index(Index)
	//{}
	Tile(Enjon::Math::Vec2 Pos, Enjon::Math::Vec2 Dims, unsigned int Index)
		: pos(Pos), dims(Dims), index(Index)
	{}

	//TODO:: Instead of giving each tile a texture, give it a Row/Col position or UV information from a specific spritesheet
	//Enjon::Graphics::GLTexture texture;
	unsigned int index;
	Enjon::Math::Vec2 pos;	
	Enjon::Math::Vec2 dims; 
};

class Level
{
public:
	Level(){}
	~Level();

	void Init(float x, float y, int row, int col);
	void DrawIsoLevel(Enjon::Graphics::SpriteBatch& batch); 
	void DrawCartesianLevel(Enjon::Graphics::SpriteBatch& batch); 

	inline int GetHeight() { return m_height; }
	inline int GetWidth() { return m_width; }
	inline Enjon::Math::Vec2 GetDims() { return Enjon::Math::Vec2(m_width, m_height); }

private:
	std::vector<Tile> m_isotiles;
	std::vector<Tile> m_cartesiantiles; 

	Enjon::Graphics::SpriteSheet m_tilesheet;

	int m_width;
	int m_height;

};




#endif