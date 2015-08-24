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
explosion <- [];
damage <- 0;
update_score( "  $000  " );
pause <- 0;
is_paused <- false;

piss_meter <- 0.0;
piss_max <- 100.0;
water_meter <-0.0;
water_max <- 100.0;
game_over <- false;
made_it <- false;
shore_line <- 0;

const screen_width = 640;
const screen_height = 480;

function generate_explosion( x, y ) {
	::is_paused = true;
	if( explosion.len() == 0 ) {
		for( local i = 0; i < 250; ++i ) {
			local particle = {
				x = 0,
				y = 0,
				c = 0,
				dx = 0.0,
				dy = 0.0,
				t = 2.0,
			};
			explosion.append( particle );
		}
	}
	foreach( i, v in explosion ) {
		v.x = x;
		v.y = y;
		v.c = 0xFF;	
		v.dx = (rand() / RAND_MAX.tofloat()) * 2.0 - 1.0;
		v.dy = ( rand() / RAND_MAX.tofloat() ) * 2.0 - 1.0;
		v.t = (rand() / RAND_MAX.tofloat()) * 2.0;
	}
	play_sound( 2 );
}

function generate_pee() {
	for( local i = 0; i < 2000; ++i ) {
		local pee = {
			x = rand() % 40 + 300,
			y = screen_height,
			dx = (rand() % 100) * 0.08 - 4.0,
			dy = (rand() % 100) * 0.4,
			c = rand() % 32 + 192,
		};	
		piss.append( pee );
	}	
	play_sound( 1 );
}

function update( current ) {

	if( background_ticks == null ) {
		background_ticks = current;
	}

	local delta = current - background_ticks;
	if( made_it ) {
		shore_line += 0.12 * delta;
		if( shore_line >= screen_height ) {
			if( !game_over ) {
				generate_pee();
			}
			game_over = true;
		}
	} 
	foreach( i, p in explosion ) {
		if( p.t > 0 ) {
			p.t -= 0.005 * delta;
			p.x += p.dx;
			p.y += p.dy;
			p.c *= 0.95;
		}
	}
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
	} 
	/*if( !made_it ){ */
	if( is_paused ) {
		pause += 0.001 * delta;
		if( pause > 1 ) {
			pause = 0;
			is_paused = false;
		}
	}

		if( current - background_ticks > 20 ) {
			delta = 20;	
		}
		piss_meter += 0.002 * delta; // <- this is the good one
		if( !made_it && piss_meter >= 100 ) {
			if( !game_over )
				generate_pee();
			game_over = true;
			return;
		}

		if( water_meter >= 100.0 ) {
			made_it = true;
		}
		
		if( is_paused ) delta = 0;
		background_delta += delta;


		water_meter += 0.0025 * delta; // <- this is the good one
		if( background_delta > 100 ) {
			background_delta -= 100;
			if( true ) next_background();
		}
		left_foot.delta = 162 * (delta / 1000.0);
		right_foot.delta = 162 * (delta / 1000.0);
		if( !left_foot.forward ) {
			if( left_foot.y > screen_height - left_foot.h ) {
				left_foot.forward = !left_foot.forward;
				play_sound( 3 );
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
				play_sound( 3 );
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

		if( cars.len() < 1 && !made_it ) {
			local car = {
				x = rand() % 350 + 100, // between 100 and 450 - w
				y = 0,
				w = 30,
				h = rand() % 5 == 0 ? 150 : 50,
			};
			car.y -= car.h;
			cars.append( car );
		}
		local asdf = damage;
		cars = cars.filter( function( i, car ) {
			car.y += 165 * delta / 1000.0;
			if( car.y > screen_height ) {
				return false;
			} else {
				if( !right_foot.forward && contains( car, right_foot ) ) {
					asdf = damage + rand() % 200 + 25;
					update_score( " -$" + asdf + "  " );
					generate_explosion( car.x + car.w, car.y + car.h );
					return false;
				}
				if( !left_foot.forward && contains( car, left_foot ) ) {
					asdf = damage + rand() % 200 + 25;
					update_score( " -$" + asdf + "  " );
					generate_explosion( car.x, car.y );
					return false;
				}
				return true;
			}
		});
		damage = asdf;
		/*}*/
}

function contains( a, b ) {
	if( a.x > b.x + b.w || b.x > a.x + a.w ) return false;
	if( a.y > b.y + b.h || b.y > a.y + a.h ) return false;
	return true;
}

function render() {

	// hide red dot
	draw_rect( 320, 320, 50, 50, 0x99, 0x99, 0x99, 0xFF );

	if( !game_over ) {

		foreach( i, car in cars ) {
			if( car.h == 50 ) {
				draw_texture( 4, car.x, car.y, car.w, car.h );
			} else {
				draw_texture( 5, car.x, car.y, car.w, car.h );
			}
		}

		draw_texture( 3, left_foot.x, left_foot.y, 100, 400 );
		draw_texture( 2, right_foot.x, right_foot.y, 100, 400 );

		if( !made_it ) { // draw water meter
			local range = (100 - water_meter) / water_max;
			local p0 = range * 150;
			local p1 = (1.0 - range) * 150;
			draw_rect( 10, 200, 30, p0, 0x33, 0x33, 0x33, 0xFF );
			draw_rect( 10 , 200 + p0, 30, p1, 0x55, 0x55, 0xFF, 0xFF );
		}

		if( !made_it ) { // draw piss meter
			local range = (100 - piss_meter) / piss_max;
			local p0 = range * 150;
			local p1 = (1.0 - range) * 150;
			draw_rect( 10, 40, 30, p0, 0x33, 0x33, 0x33, 0xFF );
			draw_rect( 10 , 40 + p0, 30, p1, 0xFF, 0xFF, 0x00, 0xFF );
		}

		foreach( i, p in explosion ) {
			if( p.t > 0 ) {
				draw_rect( p.x, p.y, 2, 2, 0xFF, p.c, p.c, 0xFF );
			}
		}

	}

	if( made_it ) {
		draw_rect( 0, 0, screen_width, shore_line, 0x55, 0x55, 0xFF, 0xFF );
	} 

	if( game_over ) {
		foreach( i, pee in piss ) {
			if( made_it ) {
				draw_rect( pee.x, pee.y, 10, 200, 0x55, 0x55, pee.c, 0xFF );

			} else {
				draw_rect( pee.x, pee.y, 10, 200, pee.c, pee.c, 0x00, 0xFF );
			}
		}
		
	} 

	// render belly
	local bx = (left_foot.x + (right_foot.x + right_foot.w) ) / 4.0;
	draw_texture( 1, bx, 300, 400, 200 );

}
