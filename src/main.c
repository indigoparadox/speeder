
#include <stdlib.h>

#define MAUG_C
#include <maug.h>

#define RETROFLT_C
#include <retroflt.h>

#define SPEEDER_OBJ_SZ_MAX 20

#define SPEEDER_RAY_ANGLE_INC 0.1f
#define SPEEDER_RAY_DEPTH_INC 0.1f
#define SPEEDER_RAY_DEPTH_MAX 20.0f

#define MINIMAP_X 10
#define MINIMAP_Y 10

struct SPEEDER_OBJ {
   int active;
   float xf;
   float zf;
   float x;
   float y;
   float z;
   float v;
   float pov_dist;
};

struct SPEEDER_DATA {
   struct SPEEDER_OBJ objects[SPEEDER_OBJ_SZ_MAX];
   float ray_inc_x;
};

struct SPEEDER_OBJ* speeder_cast_ray_x(
   struct SPEEDER_DATA* data,
   float sx, float sy, float ang1, float ang2, float depth
) {
   float x1 = 0,
      y1 = 0,
      x2 = 0,
      y2 = 0;
   int i = 0;

   /* Get coordinates for this iteration of seeking rays. */
   x1 = cos( ang1 ) * depth;
   y1 = sin( ang1 ) * depth;
   x2 = cos( ang2 ) * depth;
   y2 = sin( ang2 ) * depth;

   if( depth > SPEEDER_RAY_DEPTH_MAX ) {
      debug_printf( 0, "ray maxxed at %f, %f/%f, %f", x1, y1, x2, y2 );
      return NULL;
   }

   for( i = 1 ; SPEEDER_OBJ_SZ_MAX > i ; i++ ) {
      if( !data->objects[i].active ) {
         continue;
      }
      if(
         /* Object is top-right from source. */
         data->objects[i].x > sx && data->objects[i].y > sy &&

         data->objects[i].x < x1 && data->objects[i].x >= x2 &&
         data->objects[i].y >= y1 && data->objects[i].y < y2 
      ) {
         data->objects[i].pov_dist = depth;
         return &(data->objects[i]);
      }
   }

   return speeder_cast_ray_x(
      data, sx, sy, ang1, ang2, depth + SPEEDER_RAY_DEPTH_INC );
}

void speeder_loop_iter( struct SPEEDER_DATA* data ) {
   struct RETROFLAT_INPUT input_evt;
   int input = 0;
   int scan_x = 0;
   float ang = 0;
   struct SPEEDER_OBJ* obj = NULL;
   float depths[800];

   /* Input */

   input_evt.allow_repeat = 1;
   input = retroflat_poll_input( &input_evt );

   switch( input ) {
   case RETROFLAT_KEY_ESC:
      retroflat_quit( 0 );
      break;

   case RETROFLAT_KEY_RIGHT:
      data->objects[0].xf += 0.1;
      debug_printf( 1, "facing: %f", data->objects[0].xf );
      break;

   case RETROFLAT_KEY_LEFT:
      data->objects[0].xf -= 0.1;
      debug_printf( 1, "facing: %f", data->objects[0].xf );
      break;
   }


   /* Draw */

   retroflat_draw_lock( NULL );

   retroflat_rect(
      NULL, RETROFLAT_COLOR_BLACK, 0, 0,
      retroflat_screen_w(), retroflat_screen_h(), RETROFLAT_FLAGS_FILL );

   for( scan_x = 0 ; retroflat_screen_w() > scan_x ; scan_x += 2 ) {
      ang = data->objects[0].xf + (data->ray_inc_x * scan_x);

      obj = speeder_cast_ray_x(
         data, data->objects[0].x, data->objects[0].y,
         ang, ang + SPEEDER_RAY_ANGLE_INC, 0.1 );

      if( NULL != obj ) {
         depths[scan_x] = obj->pov_dist;
      } else {
         depths[scan_x] = 0;
      }

      if( NULL != obj ) {
         debug_printf( 0, "obj %f away", obj->pov_dist );
         retroflat_rect( NULL, RETROFLAT_COLOR_WHITE,
            scan_x, 
            (retroflat_screen_h() / 2),
            3, 3, RETROFLAT_FLAGS_FILL );
      }
   }

   /* Draw the minimap. */
   for( scan_x = 0 ; retroflat_screen_w() > scan_x ; scan_x++ ) {
      ang = data->objects[0].xf + (data->ray_inc_x * scan_x);

      if( 0 == depths[scan_x] ) {
         /* Draw a dot at the maximum range. */
         retroflat_px(
            NULL, RETROFLAT_COLOR_BLUE,
            MINIMAP_X + SPEEDER_RAY_DEPTH_MAX +
               (cos( ang ) * SPEEDER_RAY_DEPTH_MAX),
            MINIMAP_Y + SPEEDER_RAY_DEPTH_MAX +
               (sin( ang ) * SPEEDER_RAY_DEPTH_MAX), 0 );
      } else {
         /* Draw a dot at the found depth. */
         retroflat_px(
            NULL, RETROFLAT_COLOR_RED,
            MINIMAP_X + SPEEDER_RAY_DEPTH_MAX +
               (cos( ang ) * depths[scan_x]),
            MINIMAP_Y + SPEEDER_RAY_DEPTH_MAX +
               (sin( ang ) * depths[scan_x]), 0 );
      }
   }

   retroflat_draw_release( NULL );
}

int main( int argc, char** argv ) {
   int retval = 0;
   struct RETROFLAT_ARGS args;
   struct SPEEDER_DATA data;

   /* === Setup === */

   args.screen_w = 800;
   args.screen_h = 600;
   args.title = "speeder";
   args.assets_path = "";

   retval = retroflat_init( argc, argv, &args );
   if( RETROFLAT_OK != retval ) {
      goto cleanup;
   }

   memset( &data, '\0', sizeof( struct SPEEDER_DATA ) );

   /* === Main Loop === */

   data.objects[1].x = 10;
   data.objects[1].y = 4;
   data.objects[1].active = 1;
   data.ray_inc_x = RETROFLAT_PI / retroflat_screen_w();

   retroflat_loop( (retroflat_loop_iter)speeder_loop_iter, &data );

cleanup:

   retroflat_shutdown( retval );

   return retval;
}
END_OF_MAIN()

