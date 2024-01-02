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
function clockTest( arr )
{
	if( !this.clockSetup )
	{
		this.place = 0|0;
		this.clockSetup = true;
	}

	const pxouter = 393;
	const pxperdiv = 20;
	const pxperhand = 30;
	const totalpxo = pxouter + (12*pxperdiv) + 3 * pxperhand;
	var today = new Date();
	var seconds = today.getSeconds() + today.getMilliseconds()/1000.0;
	var minutes= today.getMinutes() + seconds / 60.0;
	var hours = today.getHours() + minutes/12.0;


	for( var i = 1|0; i < 254|0; i+= 2)
	{
		var j = this.place++;

		// Do this if you want to make it totally random.
		//j = Math.random() * (totalpxo);

		// Do this to add a tiny amount of randomness.
		j += Math.random()*.1;

		let x = -100;
		let y = -100;

		if( j < pxouter )
		{
			x = Math.sin( j * .016 );
			y = Math.cos( j * .016 );
		}
		else if( j < pxouter + (12*pxperdiv) )
		{
			let d = ((j - pxouter )/pxperdiv);
			let digit = d | 0;
			const shape = [
				[ 0, -1, 0, 1],
				[ -1, -1, 1, -1,   1, -1, 1, 0,   1, 0, -1, 0,   -1, 0, -1, 1,   -1, 1, 1, 1 ],
				[ -1, -1, 1, -1,   1, -1, 1, 0,   1, 0, -1, 0,   1, 0, 1, 1,   -1, 1, 1, 1 ],
				[ -1, -1, -1, 0,   1, -1, 1, 0,   1, 0, -1, 0,   1, 0, 1, 1 ],
				[ -1, -1, 1, -1,   -1, -1, -1, 0,   1, 0, -1, 0,   1, 0, 1, 1,   -1, 1, 1, 1 ],
				[ -1, 0, -1, 1,    1, -1, -1, 0,   1, 0, -1, 0,   1, 0, 1, 1,   -1, 1, 1, 1 ],
				[ -1, -1, 1, -1,    1, -1, 0, 0,   0, 0, 0, 1,],
				[ -1, -1, 1, -1,   -1, -1, -1, 0,   1, 0, -1, 0,   1, 0, 1, 1,   -1, 1, 1, 1 ,   -1, 0,-1, 1 ,   1, 0, 1,-1 ],
				[ -1, 0, -1, -1,   -1,  1, 1, 0,   1, 0, -1, 0,   1, 0, 1,-1,   -1,-1, 1,-1 ],
				[ -2, -1, -2, 1,   0, -1, 2, -1,  2,-1,2,1,      2,1,0,1,    0,1,0,-1  ],
				[ -2, -1, -2, 1,   1, -1, 1, 1 ],
				[ -2, -1, -2, 1,   0, -1, 2, -1,   2, -1, 2, 0,   2, 0, 0, 0,   0, 0, 0, 1,   0, 1, 2, 1 ],
			];
			if( digit < shape.length )
			{
				ts = shape[digit];

				let segs = (ts.length / 4)|0;

				x = Math.sin( (digit +1)/ 6.0 * 3.13159 ) * 0.8;
				y = -Math.cos( (digit +1)/ 6.0 * 3.13159 ) * 0.8;

				//let tpl = Math.random() * segs;
				let tpl = ( d % 1) * segs;

				let tseg = ( tpl ) | 0;
				console.log( tseg*4+1, ts[tseg*4+1], ts  );
				let along = tpl % 1;
				x += lerp( ts[tseg*4+0], ts[tseg*4+2], along ) * 0.05;
				y += lerp( ts[tseg*4+1], ts[tseg*4+3], along ) * 0.09;
			}
		}
		else if( j < totalpxo  )
		{
			let handpx = (j - (pxouter + 12*pxperdiv) ) / pxperhand;
			let whichhand = handpx | 0;
			let along = handpx % 1;
			let ex = 1.0;
			let ey = 1.0;
			let angle = 0.0;
			let len = 1.0;
			switch( whichhand )
			{
				case 0: angle = seconds / 30.0 * 3.14159; len = 0.65; break;
				case 1: angle = minutes / 30.0 * 3.14159; len = 0.6; break;
				case 2: angle = hours / 12.0 * 3.14159 * 2.0; len = 0.5; break;
			}
			ex = Math.cos( angle - 1.5707 );
			ey = Math.sin( angle - 1.5707 );

			x = lerp( 0, ex, along ) * len;
			y = lerp( 0, ey, along ) * len;
		}
		else
		{
			j = this.place = 0;
			arr[i+1] = 0;
			arr[i+0] = 0;	
			continue;
		}

		arr[i+1] = (x*60+ 64) & 0x7f;
		arr[i+0] = (y*60 + 64) & 0x7f;	
	}
}

function lerp( x, y, a )
{
	return x * (1.0-a) + y * a;
}

function sigrand()
{
	return Math.sqrt( -2.0 * Math.log( 1.0 - Math.random() ) ) * Math.cos( 2.0 * Math.PI * Math.random() );
}

demoFunctions = { juliaIIM, sirpinski, clockTest };