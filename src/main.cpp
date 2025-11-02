/*This source code copyrighted by Lazy Foo' Productions 2004-2025
and may not be redistributed without written permission.*/

/* Headers */
//Using SDL, SDL_image, and STL string
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <cmath>


/* Constants */
//Screen dimension constants
constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 480 };
constexpr int kScreenFps{ 60 };


/* Class Prototypes */
class LTexture
{
public:
    //Symbolic constant
    static constexpr float kOriginalSize = -1.f;

    //Initializes texture variables
    LTexture();

    //Cleans up texture variables
    ~LTexture();

    //Loads texture from disk
    bool loadFromFile( std::string path, uint8_t, uint8_t, uint8_t);

    #if defined(SDL_TTF_MAJOR_VERSION)
    //Creates texture from text
    bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
    #endif

    //Cleans up texture
    void destroy();

    //Sets color modulation
    void setColor( Uint8 r, Uint8 g, Uint8 b);

    //Sets opacity
    void setAlpha( Uint8 alpha );

    //Sets blend mode
    void setBlending( SDL_BlendMode blendMode );

    //Draws texture
    void render( float x, float y, SDL_FRect* clip = nullptr, float width = kOriginalSize, float height = kOriginalSize, double degrees = 0.0, SDL_FPoint* center = nullptr, SDL_FlipMode flipMode = SDL_FLIP_NONE );

    //Gets texture attributes
    int getWidth();
    int getHeight();
    bool isLoaded();

private:
    //Contains texture data
    SDL_Texture* mTexture;

    //Texture dimensions
    int mWidth;
    int mHeight;
};

class LTimer
{
    public:
        //Initializes variables
        LTimer();

        //The various clock actions
        void start();
        void stop();
        void pause();
        void unpause();

        //Gets the timer's time
        Uint64 getTicksNS();

        //Checks the status of the timer
        bool isStarted();
        bool isPaused();

    private:
        //The clock time when the timer started
        Uint64 mStartTicks;

        //The ticks stored when the timer was paused
        Uint64 mPausedTicks;

        //The timer status
        bool mPaused;
        bool mStarted;
};

class Dot
{
    public:
        //The dimensions of the dot
        static constexpr int kDotWidth = 20;
        static constexpr int kDotHeight = 20;

        //Maximum axis velocity of the dot
        static constexpr int kDotVel = 10;

        //Initializes the variables
        Dot(int, int);

        //Takes key presses and adjusts the dot's velocity
        void handleEvent( SDL_Event& e );

        //Moves the dot
        void move();

        //Shows the dot on the screen
        void render();

    private:
        //The X and Y offsets of the dot
        int mPosX, mPosY;

        //The velocity of the dot
        int mVelX, mVelY;

        //Dot anchor
        int mAncX, mAncY;
};


/* Function Prototypes */
//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();


/* Global Variables */
//The window we'll be rendering to
SDL_Window* gWindow{ nullptr };

//The renderer used to draw to the window
SDL_Renderer* gRenderer{ nullptr };

//Dot texture
LTexture gDotTexture; 

//The sprite sheet texture
LTexture gSpriteSheetTexture; 


/* Class Implementations */
//LTexture Implementation
LTexture::LTexture():
    //Initialize texture variables
    mTexture{ nullptr },
    mWidth{ 0 },
    mHeight{ 0 }
{

}

LTexture::~LTexture()
{
    //Clean up texture
    destroy();
}

bool LTexture::loadFromFile( std::string path, uint8_t r = 0x00, uint8_t g = 0xFF, uint8_t b = 0xFF)
{
    //Clean up texture if it already exists
    destroy();

    //Load surface
    if( SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); loadedSurface == nullptr )
    {
        SDL_Log( "Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError() );
    }
    else
    {
        //Color key image
        if( SDL_SetSurfaceColorKey( loadedSurface, true, SDL_MapSurfaceRGB( loadedSurface, r, g, b ) ) == false )
        {
            SDL_Log( "Unable to color key! SDL error: %s", SDL_GetError() );
        }
        else
        {
            //Create texture from surface
            if( mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface ); mTexture == nullptr )
            {
                SDL_Log( "Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError() );
            }
            else
            {
                //Get image dimensions
                mWidth = loadedSurface->w;
                mHeight = loadedSurface->h;
            }
        }
        
        //Clean up loaded surface
        SDL_DestroySurface( loadedSurface );
    }

    //Return success if texture loaded
    return mTexture != nullptr;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Clean up existing texture
    destroy();

    //Load text surface
    if( SDL_Surface* textSurface = TTF_RenderText_Blended( gFont, textureText.c_str(), 0, textColor ); textSurface == nullptr )
    {
        SDL_Log( "Unable to render text surface! SDL_ttf Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create texture from surface
        if( mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface ); mTexture == nullptr )
        {
            SDL_Log( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Free temp surface
        SDL_DestroySurface( textSurface );
    }
    
    //Return success if texture loaded
    return mTexture != nullptr;
}
#endif

void LTexture::destroy()
{
    //Clean up texture
    SDL_DestroyTexture( mTexture );
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

void LTexture::setColor( Uint8 r, Uint8 g, Uint8 b )
{
    SDL_SetTextureColorMod( mTexture, r, g, b );
}

void LTexture::setAlpha( Uint8 alpha )
{
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::setBlending( SDL_BlendMode blendMode )
{
    SDL_SetTextureBlendMode( mTexture, blendMode );
}

void LTexture::render( float x, float y, SDL_FRect* clip, float width, float height, double degrees, SDL_FPoint* center, SDL_FlipMode flipMode )
{
    //Set texture position
    SDL_FRect dstRect{ x, y, static_cast<float>( mWidth ), static_cast<float>( mHeight ) };

    //Default to clip dimensions if clip is given
    if( clip != nullptr )
    {
        dstRect.w = clip->w;
        dstRect.h = clip->h;
    }

    //Resize if new dimensions are given
    if( width > 0 )
    {
        dstRect.w = width;
    }
    if( height > 0 )
    {
        dstRect.h = height;
    }

    //Render texture
    SDL_RenderTextureRotated( gRenderer, mTexture, clip, &dstRect, degrees, center, flipMode );
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

bool LTexture::isLoaded()
{
    return mTexture != nullptr;
}

//LTimer Implementation
LTimer::LTimer():
    mStartTicks{ 0 },
    mPausedTicks{ 0 },

    mPaused{ false },
    mStarted{ false }
{

}

void LTimer::start()
{
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicksNS();
    mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

    //Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicksNS() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicksNS() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint64 LTimer::getTicksNS()
{
    //The actual timer time
    Uint64 time{ 0 };

    //If the timer is running
    if( mStarted )
    {
        //If the timer is paused
        if( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicksNS() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted()
{
    //Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
    //Timer is running and paused
    return mPaused && mStarted;
}

//Dot Implementation
Dot::Dot(int x, int y):
    mPosX{ x },
    mPosY{ y },
    mVelX{ 0 },
    mVelY{ 0 },
    mAncX{ mPosX },
    mAncY{ mPosY }
{

}

void Dot::handleEvent( SDL_Event& e )
{
    if( e.type == SDL_EVENT_MOUSE_MOTION) {
        if(SDL_GetMouseFocus() == nullptr) {
            mPosX = mAncX;
            mPosY = mAncY;
        }
        else {
            //Get mouse position
            float x = -1.f, y = -1.f;
            SDL_GetMouseState( &x, &y );

            const float x_anc = static_cast<float>(mAncX);
            const float y_anc = static_cast<float>(mAncY);

            float x_vec = x - x_anc;
            float y_vec = y - y_anc;
            const float z_vec = 200;

            float norm = std::sqrt(x_vec * x_vec + y_vec * y_vec + z_vec * z_vec);

            x_vec /= norm;
            y_vec /= norm; 


            mPosX = 20 * x_vec + x_anc;
            mPosY = 20 * y_vec + y_anc;
        }
    }
}

void Dot::move()
{
    //Move the dot left or right
    mPosX += mVelX;

    //If the dot went too far to the left or right
    if( ( mPosX < 0 ) || ( mPosX + kDotWidth > kScreenWidth ) )
    {
        //Move back
        mPosX -= mVelX;
    }

    //Move the dot up or down
    mPosY += mVelY;

    //If the dot went too far up or down
    if( ( mPosY < 0 ) || ( mPosY + kDotHeight > kScreenHeight ) )
    {
        //Move back
        mPosY -= mVelY;
    }
}

void Dot::render()
{
    //Show the dot
    gDotTexture.render( static_cast<float>( mPosX - kDotWidth / 2 ), static_cast<float>( mPosY - kDotHeight / 2 ) );
}

/* Function Implementations */
bool init()
{
    //Initialization flag
    bool success{ true };

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) == false )
    {
        SDL_Log( "SDL could not initialize! SDL error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window with renderer
        if( SDL_CreateWindowAndRenderer( "Buy Skyrim right now!", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) == false )
        {
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        }
    }

    return success;
}

bool loadMedia()
{
    //File loading flag
    bool success{ true };

    //Load scene textures
    if( gDotTexture.loadFromFile( "assets/dot.png", 0xFF, 0xFF, 0xFF ) == false )
    {
        SDL_Log( "Unable to dot image!\n");
        success = false;
    }

    if( gSpriteSheetTexture.loadFromFile( "assets/jokr.png" ) == false )
    {
        SDL_Log( "Unable to load sprite sheet image!\n");
        success = false;
    }

    return success;
}

void close()
{
    //Clean up textures
    gDotTexture.destroy();

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //Quit SDL subsystems
    SDL_Quit();
}


int main( int argc, char* args[] )
{
    //Final exit code
    int exitCode{ 0 };

    //Initialize
    if( init() == false )
    {
        SDL_Log( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else
    {
        //Load media
        if( loadMedia() == false )
        {
            SDL_Log( "Unable to load media!\n" );
            exitCode = 2;
        }
        else
        {
            //The quit flag
            bool quit{ false };

            //The event data
            SDL_Event e;
            SDL_zero( e );

            //Timer to cap frame rate
            LTimer capTimer;

            //Dot we will be moving around on the screen
            Dot dot1(kScreenWidth / 2 - 35, kScreenHeight / 2 - 115);
            Dot dot2(kScreenWidth / 2 + 128, kScreenHeight / 2 - 115);

            //The main loop
            while( quit == false )
            {
                //Start frame time
                capTimer.start();

                //Get event data
                while( SDL_PollEvent( &e ) == true )
                {
                    //If event is quit type
                    if( e.type == SDL_EVENT_QUIT )
                    {
                        //End the main loop
                        quit = true;
                    }

                    //Process dot events
                    dot1.handleEvent( e );
                    dot2.handleEvent( e );
                }
                
                //Update dot
                dot1.move();
                dot2.move();

                //Fill the background
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF,  0xFF );
                SDL_RenderClear( gRenderer );

                //Init sprite clip
                constexpr float kSpriteSize = 100.f;
                SDL_FRect spriteClip{ 0.f, 0.f, kSpriteSize, kSpriteSize };

                //Render dot
                dot1.render();
                dot2.render();

                //Draw original sized sprite
                gSpriteSheetTexture.render(0.f, 0.f, nullptr, kScreenWidth, kScreenHeight);

                //Update screen
                SDL_RenderPresent( gRenderer );

                //Cap frame rate
                constexpr Uint64 nsPerFrame = 1000000000 / kScreenFps; 
                Uint64 frameNs{ capTimer.getTicksNS() };
                if( frameNs < nsPerFrame )
                {
                    SDL_DelayNS( nsPerFrame - frameNs );
                }
            } 
        }
    }

    //Clean up
    close();

    return exitCode;
}
