#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include "SDL.h"
#include <vector>

/* Graphics are things that are displayed. it can be an animation
 * or a still image.
 */
class Graphic
{
    public:
        // create an animation or bitmap from memory
        Graphic(const char * buffer);

        // create an animation from memory and offset the animation
        Graphic(const char * buffer, int offset);
        ~Graphic();

        // draw to an SDL surface
        void draw(SDL_Surface * dest, SDL_Rect * destRect);
        void draw(SDL_Surface * dest, int x, int y);

        // check if the animation loaded ok
        bool isGood();

    private:
        /*  storage format:
            Uint32 GraphicType
            Uint32 StorageType
            RGB colorKey
            Header
            Image data
        */

        enum GraphicType {
            gtAnimation = 0, // spritesheet
            gtImage = 1, // still frame
        };

        enum StorageType {
            stBMP = 0,
            stPNG = 1,
        };

        typedef struct {
            Uint8 r;
            Uint8 g;
            Uint8 b;
        } RGB;

        typedef struct {
            Uint32 frameCount;
            Uint32 framesPerSecond;
            Uint32 frameWidth;
            Uint32 frameHeight;
            Uint32 imageSize; // size in bytes of the following image file
        } Header;

        SDL_Surface * m_spriteSheet;
        int m_frameCount;
        int m_fps;
        int m_offset;

        std::vector<SDL_Rect> m_spriteBounds; // boundaries of each sprite

        int currentFrame();
};

#endif

