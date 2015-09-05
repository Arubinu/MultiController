#define			XNES_UP		( 1 << 4 ) //B0000010000
#define			XNES_DOWN	( 1 << 5 ) //B0000100000
#define			XNES_LEFT	( 1 << 6 ) //B0001000000
#define			XNES_RIGHT	( 1 << 7 ) //B0010000000
#define			XNES_A		( 1 << ( controller ? 8 : 0 ) ) //( controller ? B0100000000 : B0000000001 )
#define			XNES_B		( 1 << ( controller ? 0 : 1 ) ) //( controller ? B0000000001 : B0000000010 )
#define			XNES_X		( 1 << ( controller ? 9 : -1 ) ) //( controller ? B1000000000 : B0000000000 )
#define			XNES_Y		( 1 << ( controller ? 1 : -1 ) ) //( controller ? B0000000010 : B0000000000 )
#define			XNES_R		( 1 << ( controller ? 11 : -1 ) ) //( controller ? B1000000000 : B0000000000 )
#define			XNES_L		( 1 << ( controller ? 10 : -1 ) ) //( controller ? B0000000010 : B0000000000 )
#define			XNES_START	( 1 << 3 ) //B0000001000
#define			XNES_SELECT	( 1 << 2 ) //B0000000100

#include		<NESpad.h>
#include		<SNESpad.h>

SNESpad			nintendo	= SNESpad( 3, 4, 5 ); // Strobe | Clock | Data

#include		<SoftwareSerial.h>
SoftwareSerial	BTserial( A0, A1 ); // RX | TX

#include		<SoftwareServo.h>
SoftwareServo	myservo;

bool			controller	= true;
bool			plugged		= false;
bool			onconnect	= false;

int				debug		= 2;
int				led_avr		= 13;
int				led_red		= 9;
int				led_green	= 10;
int				led_blue	= 11;

int				leds_pos	= 0;
int				leds_val[]	= { 0, 0, 0 };

int				save_val[]	= { 0, 0, 0, 0 };

void	setup()
{
	Serial.begin( 57600 );
	BTserial.begin( 9600 );

	myservo.attach( A2 );
	myservo.write( 0 );

	pinMode( debug, INPUT_PULLUP );
	pinMode( led_avr, OUTPUT );

	pinMode( led_red, OUTPUT );
	pinMode( led_green, OUTPUT );
	pinMode( led_blue, OUTPUT );

	logprintln();
	return ;
}

void	loop()
{
	int				state;
	bool			servo_change	= false;
	static int		servo_val		= 0;
	static int		save			= 0;
	static char		*num			= NULL;

	state = XNES_Controller( digitalRead( debug ) == LOW ? 2 : 1 );

	if ( plugged )
	{
		if ( save != state )
		{
			if ( state & XNES_START )
			{
				logprintln( F( " RED  GREEN  BLUE  SERVO" ) );

				logprint( right( 4, leds_val[ 0 ], &num ) );
				logprint( right( 6, leds_val[ 1 ], &num ) );
				logprint( right( 6, leds_val[ 2 ], &num ) );
				logprintln( right( 7, servo_val, &num ) );
			}

			leds_pos += ( state & XNES_RIGHT ) ? 1 : 0;
			leds_pos -= ( state & XNES_LEFT ) ? 1 : 0;

			leds_val[ leds_pos ] += ( state & XNES_UP ) ? 10 : 0;
			leds_val[ leds_pos ] -= ( state & XNES_DOWN ) ? 10 : 0;

			leds_val[ leds_pos ] = ( state & XNES_A ) ? 255 : leds_val[ leds_pos ];
			leds_val[ leds_pos ] = ( state & XNES_B ) ? 0 : leds_val[ leds_pos ];

			leds_pos = ( ( leds_pos < 0 ) ? -leds_pos + 1 : leds_pos ) % 3;
			for ( int i = 0; i < 3; ++i )
			{
				leds_val[ i ] = constrain( leds_val[ i ], 0, 255 );

				if ( state & XNES_X )
					save_val[ i ] = leds_val[ i ];
				else if ( state & XNES_Y )
					leds_val[ i ] = save_val[ i ];
				else if ( state & XNES_SELECT )
					leds_val[ i ] = random( 0, 255 );
			}

			if ( state & XNES_X )
				save_val[ 3 ] = servo_val;
			else if ( ( state & XNES_Y ) && ( servo_change = true ) )
				servo_val = save_val[ 3 ];

			analogWrite( led_red, leds_val[ 0 ] );
			analogWrite( led_green, leds_val[ 1 ] );
			analogWrite( led_blue, leds_val[ 2 ] );
		}

		if ( ( state & XNES_R ) && ( servo_change = true ) )
			servo_val += 1;
		else if ( ( state & XNES_L ) && ( servo_change = true ) )
			servo_val -= 1;

		if ( servo_change && !( servo_change = false ) )
		{
			servo_val = constrain( servo_val, 0, 180 );
			myservo.write( servo_val );
		}
	}

	save = state;
	delay( 15 );
	SoftwareServo::refresh();
	return ;
}

int		XNES_Controller( int deflog )
{
	static bool		start			= true;
	static bool		check			= true;
	static int		save			= 0;
	int				state			= nintendo.buttons() & ( ( 1 << 12 ) -1 );

	onconnect = false;
	plugged = ~state & B11110000;
	digitalWrite( led_avr, plugged ? LOW : HIGH );

	if ( check && !plugged )
	{
		save = 0;
		state = 0;
		check = false;
		leds_pos = 0;

		for ( int i = 0; i < 3; ++i )
			leds_val[ i ] = 0;

		if ( deflog >= 1 )
			logprintln( F( "Nintendo controller: false" ) );
	}
	else if ( plugged && ( !check || state != save || start ) )
	{
		if ( start && deflog == 1 )
			logprintln( F( "Nintendo controller: start" ) );

		onconnect = !check || start;
		if ( onconnect )
		{
			controller = !( state & ( B1111 << 8 ) ); //B111100000000
			if ( deflog >= 1 )
				logprintln( F( "Nintendo controller: true" ) );
		}

		save = state;
		state ^= controller ? 0 : ( B1111 << 8 );
		if ( !onconnect )
		{
			if ( deflog == 2 )
				logprintln( ( ( state | ( 1 << 12 ) ) << 1 | 1 ), BIN );
		}
		check = true;
	}
	else
		state ^= controller ? 0 : ( B1111 << 8 );

	start = false;
	return ( state );
}

char	*right( int size, int nbr, char **out )
{
	int				len				= 0;
	char			*str			= ( char * ) malloc( ( size + 1 ) * sizeof( char ) );
	char			buff[ 42 ]		= { 0 };

	itoa( nbr, buff, 10 );
	len = strlen( buff );

	memset( str, ' ', sizeof( char ) * size );
	str[ size ] = 0;

	strncpy( &str[ size - len ], ( const char * ) buff, len );

	if ( *out != NULL )
		free( *out );
	*out = str;

	return ( str );
}
