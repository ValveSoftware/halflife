  client dll readme.txt
-------------------------

This file details the structure of the half-life client dll,  and
how it communicates with the half-life game engine.


Engine callback functions:

Drawing functions:
	HSPRITE SPR_Load( char *picname );
		Loads a sprite into memory, and returns a handle to it.

	int  SPR_Frames( HSPRITE sprite );
		Returns the number of frames stored in the specified sprite.

	int  SPR_Height( HSPRITE x, int frame )
		Returns the height, in pixels, of a sprite at the specified frame.  
		Returns 0 is the frame number or the sprite handle is invalid.

	int  SPR_Width( HSPRITE x, int f )
		Returns the width, in pixels, of a sprite at the specified frame.  
		Returns 0 is the frame number or the sprite handle is invalid.

	int  SPR_Set( HSPRITE sprite, int r, int g, int b );
		Prepares a sprite about to be drawn.  RBG color values are applied to the sprite at this time.


	void  SPR_Draw( int frame, int x, int y );
		Precondition:  SPR_Set has already been called for a sprite.
		Draws the currently active sprite to the screen,  at position (x,y), where (0,0) is
		the top left-hand corner of the screen.


	void  SPR_DrawHoles( int frame, int x, int y );
		Precondition:  SPR_Set has already been called for a sprite.
		Draws the currently active sprite to the screen.  Color index #255 is treated as transparent.

	void  SPR_DrawAdditive( int frame, int x, int y );
		Precondition:  SPR_Set has already been called for a sprite.
		Draws the currently active sprite to the screen,  adding it's color values to the background.

	void  SPR_EnableScissor( int x, int y, int width, int height );
		Creates a clipping rectangle.  No pixels will be drawn outside the specified area.  Will
		stay in effect until either the next frame,  or SPR_DisableScissor is called.

	void  SPR_DisableScissor( void );
		Disables the effect of an SPR_EnableScissor call.

	int	 IsHighRes( void );
		returns 1 if the res mode is 640x480 or higher;  0 otherwise.

	int	 ScreenWidth( void );
		returns the screen width, in pixels.

	int	 ScreenHeight( void );
		returns the screen height, in pixels.

// Sound functions
	void PlaySound( char *szSound, int volume )
		plays the sound 'szSound' at the specified volume.  Loads the sound if it hasn't been cached.
		If it can't find the sound,  it displays an error message and plays no sound.

	void PlaySound( int iSound, int volume )
		Precondition:  iSound has been precached.
		Plays the sound, from the precache list.


// Communication functions
	void  SendClientCmd( char *szCmdString );
		sends a command to the server,  just as if the client had typed the szCmdString at the console.

	char *GetPlayerName( int entity_number );
		returns a pointer to a string, that contains the name of the specified client.  
		Returns NULL if the entity_number is not a client.
		

	DECLARE_MESSAGE(),  HOOK_MESSAGE()
		These two macros bind the message sending between the entity DLL and the client DLL to
		the CHud object.

		HOOK_MESSAGE( message_name )
			 This is used inside CHud::Init().  It calls into the engine to hook that message
			 from the incoming message stream.
			 Precondition:  There must be a function of name UserMsg_message_name declared
			 for CHud.  Eg,  CHud::UserMsg_Health() must be declared if you want to 
			 use HOOK_MESSAGE( Health );

		DECLARE_MESSAGE( message_name )
			For each HOOK_MESSAGE you must have an equivalent DECLARE_MESSAGE.  This creates
			a function which passes the hooked messages into the CHud object.


	HOOK_COMMAND(),  DECLARE_COMMAND()
		These two functions declare and hook console commands into the client dll.
		
		HOOK_COMMAND( char *command, command_name )
			Whenever the user types the 'command' at the console,  the function 'command_name'
			will be called.
			Precondition: There must be a function of the name UserCmd_command_name declared
			for CHud. Eg,  CHud::UserMsg_ShowScores() must be declared if you want to
			use HOOK_COMMAND( "+showscores", ShowScores );

		DECLARE_COMMAND( command_name )
			For each HOOK_COMMAND you must have an equivelant DECLARE_COMMAND.  This creates
			a function which passes the hooked commands into the CHud object.
		
