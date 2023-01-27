
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
   RETROFLAT_COLOR color;
};

struct SPEEDER_DATA {
   struct SPEEDER_OBJ objects[SPEEDER_OBJ_SZ_MAX];
   float ray_inc_x;
   float ray_inc_z;
   float fov;
   float fov_half;
   struct SPEEDER_OBJ** depths;
};

struct SPEEDER_OBJ* speeder_cast_ray_z(
   struct SPEEDER_OBJ* seeker, struct SPEEDER_OBJ* obj,
   float z_ang1, float z_ang2, float depth
) {
   float z1 = 0,
      z2 = 0;

   if( depth > SPEEDER_RAY_DEPTH_MAX ) {
      return NULL;
   }

   if(
      /* Object is above source. */
      (obj->z > seeker->z && obj->z < z1 && obj->z > z2) ||

      /* Object is below from source. */
      (obj->z < seeker->z && obj->z > z1 && obj->z < z2)
   ) {
   }
}

struct SPEEDER_OBJ* speeder_cast_ray(
   struct SPEEDER_DATA* data, struct SPEEDER_OBJ* seeker,
   float ang1, float ang2, float depth
) {
   float x1 = 0,
      y1 = 0,
      x2 = 0,
      y2 = 0;
   int i = 0,
      scan_z = 0;

   /* Get coordinates for this iteration of seeking rays. */
   x1 = seeker->x + (cos( ang1 ) * depth);
   y1 = seeker->y + (sin( ang1 ) * depth);
   x2 = seeker->x + (cos( ang2 ) * depth);
   y2 = seeker->y + (sin( ang2 ) * depth);

   if( depth > SPEEDER_RAY_DEPTH_MAX ) {
      debug_printf( 0, "ray maxxed at %f, %f/%f, %f", x1, y1, x2, y2 );
      return NULL;
   }

   for( i = 1 ; SPEEDER_OBJ_SZ_MAX > i ; i++ ) {
      if( !data->objects[i].active ) {
         continue;
      }
      if(
         /* Object is top-left from source. */
         (data->objects[i].x > seeker->x && data->objects[i].y > seeker->y &&
         data->objects[i].x < x1 && data->objects[i].x > x2 &&
         data->objects[i].y > y1 && data->objects[i].y < y2) ||

         /* Object is bottom-right from source. */
         (data->objects[i].x < seeker->x && data->objects[i].y < seeker->y &&
         data->objects[i].x > x1 && data->objects[i].x < x2 &&
         data->objects[i].y < y1 && data->objects[i].y > y2) ||

         /* Object is bottom-left from source. */
         (data->objects[i].x > seeker->x && data->objects[i].y < seeker->y &&
         data->objects[i].x <= x2 && data->objects[i].x > x1 &&
         data->objects[i].y <= y2 && data->objects[i].y > y1) ||

         /* Object is top-right from source. */
         (data->objects[i].x < seeker->x && data->objects[i].y > seeker->y &&
         data->objects[i].x >= x2 && data->objects[i].x < x1 &&
         data->objects[i].y >= y2 && data->objects[i].y < y1)
      ) {

         /* Found the X/Y, hunt down the Z. */
         #if 0
         for( scan_z = 0 ; retroflat_screen_h() > scan_z ; scan_z += 2 ) {
            z_ang1 = seeker->zf - data->fov_half + (data->ray_inc_z * scan_z);
            z_ang2 = z_ang1 + data->ray_inc_z;
#endif
               data->objects[i].pov_dist = depth;

         return &(data->objects[i]);
      }
   }

   return speeder_cast_ray(
      data, seeker, ang1, ang2, depth + SPEEDER_RAY_DEPTH_INC );
}

void speeder_loop_iter( struct SPEEDER_DATA* data ) {
   struct RETROFLAT_INPUT input_evt;
   int input = 0;
   int scan_x = 0;
   float ang = 0;
   struct SPEEDER_OBJ* obj = NULL;
   struct SPEEDER_OBJ* player = &(data->objects[0]);

   /* Input */

   input_evt.allow_repeat = 1;
   input = retroflat_poll_input( &input_evt );

   switch( input ) {
   case RETROFLAT_KEY_ESC:
      retroflat_quit( 0 );
      break;

   case RETROFLAT_KEY_RIGHT:
      player->xf += 0.1;
      debug_printf( 1, "facing: %f", player->xf );
      break;

   case RETROFLAT_KEY_LEFT:
      player->xf -= 0.1;
      debug_printf( 1, "facing: %f", player->xf );
      break;

   case RETROFLAT_KEY_UP:
      player->v += 0.1;
      debug_printf( 1, "velocity: %f", player->v );
      break;

   case RETROFLAT_KEY_DOWN:
      player->v -= 0.1;
      debug_printf( 1, "velocity: %f", player->v );
      break;
   }

   /* Move forward. */
   player->x += (cos( player->xf ) * player->v);
   player->y += (sin( player->xf ) * player->v);

   /* Draw */

   retroflat_draw_lock( NULL );

   retroflat_rect(
      NULL, RETROFLAT_COLOR_BLACK, 0, 0,
      retroflat_screen_w(), retroflat_screen_h(), RETROFLAT_FLAGS_FILL );

   for( scan_x = 0 ; retroflat_screen_w() > scan_x ; scan_x += 2 ) {
      /* Start scanlines at the far left of the FOV. */
      ang = player->xf - data->fov_half + (data->ray_inc_x * scan_x);

      obj = speeder_cast_ray(
         data, player,
         ang, ang + SPEEDER_RAY_ANGLE_INC, 0.1 );

      if( NULL != obj ) {
         data->depths[scan_x] = obj;
      } else {
         data->depths[scan_x] = NULL;
      }

      if( NULL != obj ) {
         debug_printf( 0, "obj %f away", obj->pov_dist );
         retroflat_rect( NULL, obj->color,
            scan_x, 
            (retroflat_screen_h() / 2),
            3, 3, RETROFLAT_FLAGS_FILL );
      }
   }

   /* Draw the minimap. */
   retroflat_line(
      NULL, RETROFLAT_COLOR_WHITE,
      MINIMAP_X + SPEEDER_RAY_DEPTH_MAX,
      MINIMAP_Y + SPEEDER_RAY_DEPTH_MAX,
      MINIMAP_X + SPEEDER_RAY_DEPTH_MAX + (cos( player->xf ) * 10),
      MINIMAP_Y + SPEEDER_RAY_DEPTH_MAX + (sin( player->xf ) * 10),
      0 );

   for( scan_x = 0 ; retroflat_screen_w() > scan_x ; scan_x += 2 ) {
      /* Start scanlines at the far left of the FOV. */
      ang = player->xf - data->fov_half + (data->ray_inc_x * scan_x);

      if( NULL == data->depths[scan_x] ) {
         /* Draw a dot at the maximum range. */
         retroflat_px(
            NULL, RETROFLAT_COLOR_DARKGRAY,
            MINIMAP_X + SPEEDER_RAY_DEPTH_MAX +
               (cos( ang ) * SPEEDER_RAY_DEPTH_MAX),
            MINIMAP_Y + SPEEDER_RAY_DEPTH_MAX +
               (sin( ang ) * SPEEDER_RAY_DEPTH_MAX), 0 );
      } else {
         /* Draw a dot at the found depth. */
         retroflat_px(
            NULL, data->depths[scan_x]->color,
            MINIMAP_X + SPEEDER_RAY_DEPTH_MAX +
               (cos( ang ) * data->depths[scan_x]->pov_dist),
            MINIMAP_Y + SPEEDER_RAY_DEPTH_MAX +
               (sin( ang ) * data->depths[scan_x]->pov_dist), 0 );
      }
   }

   retroflat_draw_release( NULL );
}

int main( int argc, char** argv ) {
   int retval = 0;
   struct RETROFLAT_ARGS args;
   struct SPEEDER_DATA data;

   /* === Setup === */

   args.screen_w = 320;
   args.screen_h = 240;
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
   data.objects[1].color = RETROFLAT_COLOR_RED;

   data.objects[2].x = -8;
   data.objects[2].y = -10;
   data.objects[2].active = 1;
   data.objects[2].color = RETROFLAT_COLOR_GREEN;

   data.objects[3].x = 5;
   data.objects[3].y = -4;
   data.objects[3].active = 1;
   data.objects[3].color = RETROFLAT_COLOR_BLUE;

   data.objects[4].x = -3;
   data.objects[4].y = 3;
   data.objects[4].active = 1;
   data.objects[4].color = RETROFLAT_COLOR_CYAN;

   /* Full circle is 2pi, so pi=180 degrees, so pi/2=90 degree FOV. */
   data.ray_inc_x = (RETROFLAT_PI / 2) / retroflat_screen_w();
   data.ray_inc_z = (RETROFLAT_PI / 2) / retroflat_screen_h();
   data.depths = calloc( retroflat_screen_w(), sizeof( struct SPEEDER_OBJ* ) );
   data.fov_half = RETROFLAT_PI / 4;
   assert( NULL != data.depths );

   retroflat_loop( (retroflat_loop_iter)speeder_loop_iter, &data );

cleanup:

#ifndef RETROFLAT_OS_WASM

   if( NULL != data.depths ) {
      free( data.depths );
   }

#endif /* !RETROFLAT_OS_WASM */

   retroflat_shutdown( retval );

   return retval;
}
END_OF_MAIN()

