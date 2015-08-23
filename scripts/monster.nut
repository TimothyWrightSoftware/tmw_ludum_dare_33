println( "asdfWoohoo, loaded the script!!!" );

background_delta <- 0;
background_ticks <- null;
background_frame <- 0;

left_foot <- {
	x = 200,
	  y = 300,
	  w = 100,
	  h = 100,
	  forward = false,
	  delta = 0,
}

right_foot <- {
	x = 350,
	  y = 350,
	  w = 100,
	  h = 100,
	  forward = true,
	  delta = 0,
}

cars <- [];
piss <-[];
damage <- 123;
update_score( "  $000  " );

piss_meter <- 0.0;
piss_max <- 100.0;
game_over <- false;

const screen_width = 640;
const screen_height = 480;

function update( current ) {

	if( background_ticks == null ) {
		background_ticks = current;
	}
	local delta = current - background_ticks;
	if( game_over ) {
		piss = piss.filter( function( i, pee ) {
			pee.x += pee.dx;
			pee.y -= pee.dy;	
			pee.dx *= 0.99;
			pee.dy *= 0.95;
			if( pee.x < 0 || pee.x > screen_width )
				pee.dx = -pee.dx
			/*return pee.x > 0 && pee.x < screen_width && pee.y >0 && pee.y < screen_height;*/
			return true;
		});
	} else { 
		if( current - background_ticks > 20 ) {
			delta = 20;	
		}
		/*piss_meter += 0.0020 * delta;*/ // <- this is the good one
		piss_meter += 0.220 * delta;
		if( piss_meter >= 100 ) {
			game_over = true;
			for( local i = 0; i < 1000; ++i ) {
				local pee = {
					x = rand() % 40 + 300,
					y = screen_height,
					dx = (rand() % 100) * 0.04 - 2.0,
					dy = (rand() % 100) * 0.4,
					c = rand() % 32 + 192,
				};	
				piss.append( pee );
			}	
			return;
		}
		background_delta += delta;
		if( background_delta > 100 ) {
			background_delta -= 100;
			if( true ) next_background();
		}
		left_foot.delta = 162 * (delta / 1000.0);
		right_foot.delta = 162 * (delta / 1000.0);
		if( !left_foot.forward ) {
			if( left_foot.y > screen_height - left_foot.h ) {
				left_foot.forward = !left_foot.forward;
			} else {
				left_foot.y += left_foot.delta;
			}
		}
		if( left_foot.forward ) {
			if( left_foot.y < screen_height - left_foot.h * 2 ) {
				left_foot.forward = !left_foot.forward;
			} else {
				left_foot.y -= left_foot.delta;
			}
		}
		if( !right_foot.forward ) {
			if( right_foot.y > screen_height - right_foot.h ) {
				right_foot.forward = !right_foot.forward;
			} else {
				right_foot.y += right_foot.delta;
			}
		}
		if( right_foot.forward ) {
			if( right_foot.y < screen_height - right_foot.h * 2 ) {
				right_foot.forward = !right_foot.forward;
			} else {
				right_foot.y -= right_foot.delta;
			}
		}
		background_ticks = current;

		local space =3;
		local move = 150;
		local to_far = 80;

		if( move_left ) { // move left

			if( left_foot.forward ) {	
				left_foot.x -= move * delta / 1000.0;
				if( left_foot.x < 100 ) 
					left_foot.x = 100;
				if( left_foot.x + left_foot.x + to_far < right_foot.x )
					left_foot.x = right_foot.x - to_far - left_foot.w;

				if( left_foot.x + left_foot.w + to_far < right_foot.x )
					left_foot.x = right_foot.x - to_far - left_foot.w;
			}

			if( right_foot.forward ) {	
				right_foot.x -= move * delta / 1000.0;
				if( right_foot.x < left_foot.x + left_foot.w + space )
					right_foot.x = left_foot.x + left_foot.w + space;
			}

		}

		if( move_right ) { // move right
			if( right_foot.forward ) {
				right_foot.x += move * delta / 1000.0;
				if( right_foot.x > 450 ) 
					right_foot.x = 450;
			}
			if( left_foot.forward ) {
				left_foot.x += move * delta / 1000.0;
				if( left_foot.x + left_foot.w + space > right_foot.x )
					left_foot.x = right_foot.x - space - left_foot.w;
			}
		}

		if( cars.len() < 1 ) {
		local car = {
			x = rand() % 350 + 100, // between 100 and 450 - w
			y = 0,
			w = 30,
			h = rand() % 5 == 0 ? 150 : 50,
			r = 0xFF,
			g = 0xFF,
			b = 0xFF,
		};
		car.y -= car.h;
		cars.append( car );
	}
	local asdf = damage;
	cars = cars.filter( function( i, car ) {
			car.y += 139 * delta / 1000.0;
			if( car.y > screen_height ) {
			return false;
			} else {
			if( contains( car, right_foot ) ) {
			asdf = damage + 100;
			update_score( " -$" + asdf + "  " );
			return false;
			}
			if( contains( car, left_foot ) ) {
			asdf = damage + 100;
			update_score( " -$" + asdf + "  " );
			return false;
			}
			return true;
			}
			});
	damage = asdf;
	}
}

function contains( a, b ) {
	if( a.x > b.x + b.w || b.x > a.x + a.w ) return false;
	if( a.y > b.y + b.h || b.y > a.y + a.h ) return false;
	return true;
}

function render() {

	
	// hide red dot
	draw_rect( 320, 320, 50, 50, 0x99, 0x99, 0x99, 0xFF );

	if( game_over ) {
		foreach( i, pee in piss ) {
			draw_rect( pee.x, pee.y, 10, 200, pee.c, pee.c, 0x00, 0xFF );
		}
		
	} else { 
		draw_rect( left_foot.x, left_foot.y, left_foot.w, left_foot.h, 0xFF, 0xFF, 0x00, 0x00 );
		draw_rect( right_foot.x, right_foot.y, right_foot.w, right_foot.h, 0x00, 0xFF, 0x00, 0x00 );

		foreach( i, car in cars ) {
			draw_rect( car.x, car.y, car.w, car.h, car.r, car.g, car.b, 0xFF );
		}

		{ // draw piss meter
			local range = (100 - piss_meter) / piss_max;
			local p0 = range * 150;
			local p1 = (1.0 - range) * 150;
			draw_rect( 10, 40, 30, p0, 0x33, 0x33, 0x33, 0xFF );
			draw_rect( 10 , 40 + p0, 30, p1, 0xFF, 0xFF, 0x00, 0xFF );
		}
	}
}
