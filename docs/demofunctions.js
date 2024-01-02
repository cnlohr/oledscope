function juliaIIM( arr )
{
	if( !this.juliaSetup )
	{
		this.zreal = 0.1;
		this.zimag = 0.1;
		this.step = 0|0;
		this.juliaSetup = true;
	}

	var j = this.step++;
	var creal = Math.sin( j/100.0 )*1.0;
	var cimag = Math.cos( j/100.0 )*1.0;
	var zreal = this.zreal;
	var zimag = this.zimag;

	for( var i = 1|0; i < 254|0; i+= 2)
	{
		var real = zreal - creal;
		var imag = zimag - cimag;

		// Take square root of real & imag
		var mag = Math.sqrt( real * real + imag * imag );
		zreal =                         Math.sqrt( ( mag + real ) / 2.0 );
		zimag =  ((imag>=0)?1.0:-1.0) * Math.sqrt( ( mag - real ) / 2.0 );
		
		// Randomly pick one of the roots.
		if( Math.random() > 0.5)
		{
			zreal = -zreal;
			zimag = -zimag;
		}

		arr[i+0] = (zreal*32 + 64) & 0x7f;
		arr[i+1] = (zimag*32 + 64) & 0x7f;	
	}

	this.zreal = zreal;
	this.zimag = zimag;
}

function sirpinski( arr )
{
	if( !this.sirpSetup)
	{
		this.sirpx= 0.1;
		this.sirpy = 0.1;
		this.step = 0|0;
		this.sirpSetup = true;
	}

	var j = this.step++;
	var sirpx = this.sirpx;
	var sirpy = this.sirpy;

	for( var i = 1|0; i < 254|0; i+= 2)
	{
		// https://www.johndcook.com/blog/2017/07/08/the-chaos-game-and-the-sierpinski-triangle/
		// Sirpinski's gasket
		let k = (Math.random()*3)|0;
		const corners = [ 0, 0, 0.5, 0.866, 1, 0 ];
		sirpx = ( sirpx + corners[k*2+0] ) * 0.5;
		sirpy = ( sirpy + corners[k*2+1] ) * 0.5;

		arr[i+0] = (sirpx*127) & 0x7f;
		arr[i+1] = (sirpy*127 + 9) & 0x7f;
	}

	this.sirpx = sirpx;
	this.sirpy = sirpy;
}

demoFunctions = { juliaIIM, sirpinski };