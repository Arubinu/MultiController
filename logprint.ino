void	logprintln( void )
{
	Serial.println();
	BTserial.println();
}

void	logprintln( char *str )
{
	logprint( str );
	logprintln();

	return;
}

void	logprint( char *str )
{
	Serial.print( str );
	BTserial.print( str );

	return ;
}

void	logprintln( String str )
{
	logprint( str );

	Serial.println();
	BTserial.println();

	return;
}

void	logprint( String str )
{
	Serial.print( str );
	BTserial.print( str );

	return ;
}

void	logprintln( long val )
{
	logprintln( val, -1 );

	return;
}

void	logprint( long val )
{
	logprint( val, -1 );
	return;
}

void	logprintln( long val, int type )
{
	logprint( val, type );

	Serial.println();
	BTserial.println();

	return;
}

void	logprint( long val, int type )
{
	if ( type > 0 )
	{
		Serial.print( val, type );
		BTserial.print( val, type );
	}
	else
	{
		Serial.print( val );
		BTserial.print( val );
	}

	return ;
}
