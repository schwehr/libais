// STM Route Exchange

#include <cmath>

#include "ais.h"
#include "string"

namespace libais {

Ais8_265_1::Ais8_265_1(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), num_legs(0){

  assert(dac == 265);
  assert(fi == 1);

  if (!CheckStatus()) {
    return;
  }


  bits.SeekTo(56);
  
  /*
  	Different switches depending on the message bitsize
	56:   parameters
	56:   first waypoint
	n*64: number of legs
	66:   final reported leg
	6:    steering mode and spare

	waypoint type: 0=away from first waypoint,1=towards first waypoint
	first waypoint: longitude,latitude [1/100000 min] -> [1/600000] (as done in ToAisPoint)  
	leg geometry: 0=Loxodrome,1=Orthodrome 
	speed over ground (sog): 1/10 [knots]
	turn radius: 1/100 [NM] (0=unavailabe)
	lon delta: 1/100000 [min] -> [1/600000]
	lat delta: 1/100000 [min] -> [1/600000]
  */
  status = AIS_OK;

  if (num_bits == 56){
  		num_legs=-1;
  } else if (num_bits == 56+56+0*64+66+6){

	  	// first waypoint
		first_waypoint_type = bits[56];
		position1 = bits.ToAisPoint(57,55);
		num_legs=0;
  } else if (num_bits == 56+56+1*64+66+6){
		// first waypoint
		first_waypoint_type = bits[56];
		position1 = bits.ToAisPoint(57,55);


		int current_bit = 112;
		// leg 1
		leg_geometry1 = bits[current_bit];
		current_bit++;
		sog1 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius1 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
		lat_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;

		// final reported leg
		leg_geometry_fin = bits[current_bit];
		current_bit++;
		sog_fin = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		position_fin = bits.ToAisPoint(current_bit,55);
		current_bit+=55;
		
		// steering mode and spare
		steering_mode = bits.ToUnsignedInt(current_bit,2);
		current_bit+=2;
		spare = bits.ToInt(current_bit,4);
		num_legs=1;  	

  } else if (num_bits == 56+56+2*64+66+6){
  		first_waypoint_type = bits[56];
  		position1 = bits.ToAisPoint(57,55);


  		int current_bit = 112;
  		// leg 1
		leg_geometry1 = bits[current_bit];
		current_bit++;
		sog1 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius1 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta1 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 2
	    leg_geometry2 = bits[current_bit];
		current_bit++;
		sog2 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius2 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta2 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta2 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // final reported leg
  		leg_geometry_fin = bits[current_bit];
  		current_bit++;
  		sog_fin = bits.ToUnsignedInt(current_bit, 10)/10.0;
  		current_bit+=10;
  		position_fin = bits.ToAisPoint(current_bit,55);
  		current_bit+=55;
  		
  		// steering mode and spare
  		steering_mode = bits.ToUnsignedInt(current_bit,2);
  		current_bit+=2;
  		spare = bits.ToInt(current_bit,4);

  		num_legs=2;


  } else if (num_bits == 56+56+3*64+66+6){
  		first_waypoint_type = bits[56];
  		position1 = bits.ToAisPoint(57,55);


  		int current_bit = 112;
  		// leg 1
		leg_geometry1 = bits[current_bit];
		current_bit++;
		sog1 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius1 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta1 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 2
	    leg_geometry2 = bits[current_bit];
		current_bit++;
		sog2 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius2 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta2 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta2 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;
 		
 		// leg 3
	    leg_geometry3 = bits[current_bit];
		current_bit++;
		sog3 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius3 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta3 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta3 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // final reported leg
  		leg_geometry_fin = bits[current_bit];
  		current_bit++;
  		sog_fin = bits.ToUnsignedInt(current_bit, 10)/10.0;
  		current_bit+=10;
  		position_fin = bits.ToAisPoint(current_bit,55);
  		current_bit+=55;
  		
  		// steering mode and spare
  		steering_mode = bits.ToUnsignedInt(current_bit,2);
  		current_bit+=2;
  		spare = bits.ToInt(current_bit,4);

  		num_legs=3;


  } else if (num_bits == 56+56+4*64+66+6){
  		first_waypoint_type = bits[56];
  		position1 = bits.ToAisPoint(57,55);


  		int current_bit = 112;
  		// leg 1
		leg_geometry1 = bits[current_bit];
		current_bit++;
		sog1 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius1 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta1 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 2
	    leg_geometry2 = bits[current_bit];
		current_bit++;
		sog2 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius2 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta2 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta2 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 3
	    leg_geometry3 = bits[current_bit];
		current_bit++;
		sog3 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius3 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta3 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta3 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 4
	    leg_geometry4 = bits[current_bit];
		current_bit++;
		sog4 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius4 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta4 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta4 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // final reported leg
  		leg_geometry_fin = bits[current_bit];
  		current_bit++;
  		sog_fin = bits.ToUnsignedInt(current_bit, 10)/10.0;
  		current_bit+=10;
  		position_fin = bits.ToAisPoint(current_bit,55);
  		current_bit+=55;
  		
  		// steering mode and spare
  		steering_mode = bits.ToUnsignedInt(current_bit,2);
  		current_bit+=2;
  		spare = bits.ToInt(current_bit,4);

  		num_legs=4;


  } else if (num_bits == 56+56+5*64+66+6){
  		first_waypoint_type = bits[56];
  		position1 = bits.ToAisPoint(57,55);


  		int current_bit = 112;
  		// leg 1
		leg_geometry1 = bits[current_bit];
		current_bit++;
		sog1 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius1 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta1 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 2
	    leg_geometry2 = bits[current_bit];
		current_bit++;
		sog2 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius2 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta2 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta2 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 3
	    leg_geometry3 = bits[current_bit];
		current_bit++;
		sog3 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius3 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta3 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta3 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 4
	    leg_geometry4 = bits[current_bit];
		current_bit++;
		sog4 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius4 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta4 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta4 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 5
	    leg_geometry5 = bits[current_bit];
		current_bit++;
		sog5 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius5 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta5 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta5 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // final reported leg
  		leg_geometry_fin = bits[current_bit];
  		current_bit++;
  		sog_fin = bits.ToUnsignedInt(current_bit, 10)/10.0;
  		current_bit+=10;
  		position_fin = bits.ToAisPoint(current_bit,55);
  		current_bit+=55;
  		
  		// steering mode and spare
  		steering_mode = bits.ToUnsignedInt(current_bit,2);
  		current_bit+=2;
  		spare = bits.ToInt(current_bit,4);

  		num_legs=5;


  } else if (num_bits == 56+56+6*64+66+6){
  		// first waypoint
  		first_waypoint_type = bits[56];
  		position1 = bits.ToAisPoint(57,55);


  		int current_bit = 112;
  		// leg 1
		leg_geometry1 = bits[current_bit];
		current_bit++;
		sog1 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius1 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta1 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta1 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 2
	    leg_geometry2 = bits[current_bit];
		current_bit++;
		sog2 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius2 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta2 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta2 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 3
	    leg_geometry3 = bits[current_bit];
		current_bit++;
		sog3 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius3 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta3 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta3 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 4
	    leg_geometry4 = bits[current_bit];
		current_bit++;
		sog4 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius4 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta4 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta4 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 5
	    leg_geometry5 = bits[current_bit];
		current_bit++;
		sog5 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius5 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta5 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta5 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;

	    // leg 6
	    leg_geometry6 = bits[current_bit];
		current_bit++;
		sog6 = bits.ToUnsignedInt(current_bit, 10)/10.0;
		current_bit+=10;
		turn_radius6 = bits.ToUnsignedInt(current_bit,9)/100.0;
		current_bit+=9;
		lng_delta6 = bits.ToInt(current_bit,22)/600000.0;
		current_bit+=22;
	    lat_delta6 = bits.ToInt(current_bit,22)/600000.0;
	    current_bit+=22;


  		// final reported leg
  		leg_geometry_fin = bits[current_bit];
  		current_bit++;
  		sog_fin = bits.ToUnsignedInt(current_bit, 10)/10.0;
  		current_bit+=10;
  		position_fin = bits.ToAisPoint(current_bit,55);
  		current_bit+=55;
  		
  		// steering mode and spare
  		steering_mode = bits.ToUnsignedInt(current_bit,2);
  		current_bit+=2;
  		spare = bits.ToInt(current_bit,4);

  		num_legs=6;


  } else{
  	status = AIS_ERR_BAD_BIT_COUNT;
  }
  
}


}  // namespace libais
