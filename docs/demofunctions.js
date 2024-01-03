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

	const pxouter = 30;
	const pxperdiv = 20;
	const pxperhand = 20;
	const totalpxo = pxouter + (12*pxperdiv) + 3 * pxperhand;
	var today = new Date();
	var seconds = today.getSeconds() + today.getMilliseconds()/1000.0;	
	var minutes= today.getMinutes() + seconds / 60.0;
	var hours = today.getHours() + minutes/60.0;


	for( var i = 1|0; i < 254|0; i+= 2)
	{
		var j = this.place++;

		// Do this if you want to make it totally random.
		//j = Math.random() * (totalpxo);

		// Do this to add a tiny amount of randomness.
		j += Math.random()*.5;

		let x = -100;
		let y = -100;

		if( j < pxouter )
		{
			x = Math.sin( (j|0) * 3.14159 * 2.0 / pxouter - seconds/60.0*3.14159*2.0 );
			y = Math.cos( (j|0) * 3.14159 * 2.0 / pxouter - seconds/60.0*3.14159*2.0 );
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
			let ilen = 0.0;
			switch( whichhand )
			{
				case 0: angle = seconds / 30.0 * 3.14159; len = 0.9; ilen = 1.05; break;
				case 1: angle = minutes / 30.0 * 3.14159; len = 0.6; break;
				case 2: angle = hours / 12.0 * 3.14159 * 2.0; len = 0.45; break;
			}
			ex = Math.cos( angle - 1.5707 );
			ey = Math.sin( angle - 1.5707 );

			x = lerp( ex*ilen, ex*len, along );
			y = lerp( ey*ilen, ey*len, along );
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

function streaktest( arr )
{
	if( !this.streakSetup )
	{
		this.place = 0|0;
		this.streakSetup = true;
	}

	for( var i = 1|0; i < 254|0; i+= 2)
	{
		var j = this.place++;
		arr[i+1] = (j) & 0x7f;
		arr[i+0] = ((j>>7)*8) & 0x7f;	
	}
}

function starfield( arr )
{
	if( !this.starfieldSetup )
	{
		this.starcount = 64;
		this.place = 0|0;
		this.starfieldSetup = true;
		this.stars = [];
		for( var tsx = 0; tsx < this.starcount; tsx++ )
		{
			let st = { x : stablerand(tsx), y : stablerand(tsx+9999), age : 0 };
			this.stars.push( st );
		}
	}

	let st = 0;
	for( var i = 1|0; i < 254|0; i+= 2)
	{
		let j = this.place++;
		let star = this.stars[j % this.starcount];
		var x,y;

		x = star.x * (star.age+1);
		y = star.y * (star.age+1);
		if( x > 1 || y > 1 || x < -1 || y < -1 || ( star.x * star.x + star.y * star.y ) < .01 )
		{
			star.x = stablerand(star.y+star.age);
			star.y = stablerand(star.x+star.age+9999);
			star.age = 0;
		}
		star.age+=0.02;
		arr[i+1] = (x*60+ 64) & 0x7f;
		arr[i+0] = (y*60 + 64) & 0x7f;	
	}
}

function fireworks( arr )
{
	if( !this.fireworksSetup )
	{
		this.place = 0|0;
		this.place = 0|0;
		this.fireworksSetup = true;
	}

	const numWorks = 40;
	const boom = 500;
	const life = 1700;

	let st = 0;
	for( var i = 1|0; i < 254|0; i+= 2)
	{	
		let j = this.place++;

		let stage = j % numWorks;
		let frame = (j / numWorks) % life;
		let lifetime = (((j / numWorks)|0) / life)|0;

		let x = 0;
		let y = 0;

		// All together, was launched from ground.
		let gx = stablerand( lifetime );
		let gxl = stablerand( lifetime );
		let gy = stablerand( lifetime ) - 1.0;

		let coretime = frame;
		if( coretime > boom ) coretime = boom;

		let corex = gx * 0.3 + gxl * coretime * 0.0005;
		let corey = (coretime*coretime) * 0.00001 + coretime * .0011 * (gy-5) + 1.0;

		if( frame < boom )
		{
			x = corex;
			y = corey;
		}
		else
		{
			let srx = stablerand( stage + lifetime*30 );
			let sry = stablerand( stage + lifetime*30 + 512 );
			let srz = stablerand( stage + lifetime*30 + 1024 );
			let norm = Math.sqrt( srx*srx+sry*sry+srz*srz );
			srx /= norm;
			sry /= norm;
			corey += (frame-boom)*(frame-boom)*0.000004;
			x = corex + srx*(frame-boom)*0.002;
			y = corey + (sry - 1 ) * (frame-boom)*0.002;

			//let maskrand = Math.sin( ( stablerand( stage ) * 0.1 + 0.1 ) * ( frame ) );
			//let sparkage = (frame-boom)/(life);
			//if( sparkage > maskrand ) x = -10;
		}

		if( x < -1 || y < -1 || x > 1 || y > 1 )
		{
			arr[i+1] = 0;
			arr[i+0] = 0;	
		}
		else
		{
			arr[i+1] = (x*60+ 64) & 0x7f;
			arr[i+0] = (y*60 + 64) & 0x7f;	
		}
	}
}

function stablerand( p )
{
	p = (p * .1031) % 1;
	p *= p + 33.33;
	p *= p + p;
	return (((p) % 1) - 0.5) * 2.0;
}




function lerp( x, y, a )
{
	return x * (1.0-a) + y * a;
}

function sigrand()
{
	return Math.sqrt( -2.0 * Math.log( 1.0 - Math.random() ) ) * Math.cos( 2.0 * Math.PI * Math.random() );
}

demoFunctions = { juliaIIM, sirpinski, clockTest, streaktest, starfield, fireworks };

