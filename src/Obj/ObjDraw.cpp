#include "ObjDraw.h"
#include "XObjDefs.h"
#include <gl.h>

#define	CHECK_GL_ERR		GL_ERR(glGetError());

static void GL_ERR(int err)
{
	if (err != 0)
		printf("GL ERROR: %d\n", err);
}

static void	Default_SetupPoly(void * ref)
{
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
}
static void 	Default_SetupLine(void * ref)
{
	glDisable(GL_TEXTURE_2D);
}
static void	Default_SetupLight(void * ref)
{
	glDisable(GL_TEXTURE_2D);
}
static void	Default_SetupMovie(void * ref)
{
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
}
static void	Default_SetupPanel(void * ref)
{
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
}
static void	Default_TexCoord(const float * st, void * ref)
{
	glTexCoord2fv(st);
}
	
static void Default_TexCoordPointer(int size, unsigned long type, long stride, const void * pointer, void * ref)
{
	glTexCoordPointer(size, type, stride, pointer);
}

static float	Default_GetAnimParam(const char * string, float v1, float v2, void * ref)
{
	return 0.0;
}

static	ObjDrawFuncs_t sDefault = { 
	Default_SetupPoly, Default_SetupLine, Default_SetupLight,
	Default_SetupMovie, Default_SetupPanel, Default_TexCoord, Default_TexCoordPointer, Default_GetAnimParam
};

enum {
	drawMode_Non,
	drawMode_Tri,
	drawMode_Lin,
	drawMode_Lgt,
	drawMode_Pan,
	drawMode_Mov
};

enum {
	arrayMode_Non,
	arrayMode_Tri,
	arrayMode_Lin,
	arrayMode_Lgt
};

void	ObjDraw(const XObj& obj, float dist, ObjDrawFuncs_t * funcs, void * ref)
{
	if (funcs == NULL) funcs = &sDefault;
	int 	drawMode = drawMode_Non;
	bool	do_draw = true;
	
	for (vector<XObjCmd>::const_iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		int	want_mode = drawMode_Non;
		
		if (cmd->cmdID == attr_LOD)
			do_draw = dist >= cmd->attributes[0] && dist <= cmd->attributes[1];
		
		if (do_draw)
		{
			switch(cmd->cmdID) {
			case obj_Light:				want_mode = drawMode_Lgt;	break;
			case obj_Line:				want_mode = drawMode_Lin;	break;
			case obj_Tri:
			case obj_Quad:
			case obj_Quad_Hard:
			case obj_Polygon:
			case obj_Quad_Strip:
			case obj_Tri_Strip:
			case obj_Tri_Fan:			want_mode = drawMode_Tri;	break;
			case obj_Quad_Cockpit:		want_mode = drawMode_Pan;	break;
			case obj_Movie:				want_mode = drawMode_Mov;	break;
			}
			if (want_mode != drawMode_Non)
			{
				if (want_mode != drawMode)
				{			
					switch(want_mode) {
					case drawMode_Tri:		funcs->SetupPoly_f(ref);	break;
					case drawMode_Lin:		funcs->SetupLine_f(ref);	break;
					case drawMode_Lgt:		funcs->SetupLight_f(ref);	break;
					case drawMode_Pan:		funcs->SetupPanel_f(ref);	break;
					case drawMode_Mov:		funcs->SetupMovie_f(ref);	break;
					}
				}			
				drawMode = want_mode;
			}
			
			switch(cmd->cmdType) {
			case type_Poly:
				switch(cmd->cmdID) {
				case obj_Tri:				glBegin(GL_TRIANGLES);	break;
				case obj_Quad:				
				case obj_Quad_Hard:
				case obj_Quad_Cockpit:		
				case obj_Movie:				glBegin(GL_QUADS);	break;
				case obj_Polygon:			glBegin(GL_POLYGON);	break;
				case obj_Quad_Strip:		glBegin(GL_QUAD_STRIP);	break;
				case obj_Tri_Strip:			glBegin(GL_TRIANGLE_STRIP);	break;
				case obj_Tri_Fan:			glBegin(GL_TRIANGLE_FAN);	break;
				default:					glBegin(GL_TRIANGLES);	break;
				}
				for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
				{
					funcs->TexCoord_f(st->st,ref);
					glVertex3fv(st->v);
				}
				glEnd();
				break;
			case type_PtLine:
				switch(cmd->cmdID) {
				case obj_Line:				glBegin(GL_LINES);	break;
				case obj_Light:				
				default:					glBegin(GL_POINTS);	break;
				}
				for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
				{
					glColor3f(rgb->rgb[0] * 0.1, rgb->rgb[1] * 0.1, rgb->rgb[2] * 0.1);
					glVertex3fv(rgb->v);
				}
				glEnd();
				break;
			case type_Attr:
				switch(cmd->cmdID) {
				case attr_Shade_Flat:	glShadeModel(GL_FLAT); break;
				case attr_Shade_Smooth: glShadeModel(GL_SMOOTH); break;
				case attr_Ambient_RGB: { float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_AMBIENT ,rgba);	} break;
				case attr_Diffuse_RGB: { float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_DIFFUSE ,rgba);	} break;
				case attr_Emission_RGB:{ float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_EMISSION,rgba);	} break;
				case attr_Specular_RGB:{ float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_SPECULAR,rgba);	} break;
				case attr_Shiny_Rat:	glMaterialf (GL_FRONT,GL_SHININESS,cmd->attributes[0]); break;
				case attr_No_Depth:		glDisable(GL_DEPTH_TEST);
				case attr_Depth:		glEnable(GL_DEPTH_TEST);
				case attr_Reset: { float amb[4] = { 0.2, 0.2, 0.2, 1.0 }, diff[4] = { 0.8, 0.8, 0.8, 1.0 }, zero[4] = { 0.0, 0.0, 0.0, 1.0 };
									glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
									glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
									glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
									glMaterialfv(GL_FRONT, GL_EMISSION, zero);
									glMaterialf (GL_FRONT,GL_SHININESS,0.0); } break;
				case attr_Cull:		glEnable(GL_CULL_FACE);		break;
				case attr_NoCull:	glDisable(GL_CULL_FACE);	break;
				case attr_Offset:	if (cmd->attributes[0] != 0)
					{	glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(-5.0*cmd->attributes[0],-1.0);glDepthMask(GL_FALSE);	} else
					{	glDisable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(0.0, 0.0);glDepthMask(GL_TRUE);	} 
					break;
				}
				break;
			} // Case for type of cmd
		} // Drwaing enabled
	} // While loop on cmds
}

inline float rescale_float(float x1, float x2, float y1, float y2, float x)
{
	if (x1 == x2) return y1;
	return y1 + ((y2 - y1) * (x - x1) / (x2 - x1));
}

void	ObjDraw8(const XObj8& obj, float dist, ObjDrawFuncs_t * funcs, void * ref)
{
	if (funcs == NULL) funcs = &sDefault;
	int 	drawMode = drawMode_Non;
	int		arrayMode = arrayMode_Non;
	bool	tex_is_cockpit = false;
	int 	want_draw;
	const XObjAnim8 * anim;
	
	float v;

	const XObjLOD8 * our_lod = &obj.lods.back();
	for (vector<XObjLOD8>::const_iterator lod = obj.lods.begin(); lod != obj.lods.end(); ++lod)
	if (lod->lod_near <= dist && lod->lod_far >= dist)
		our_lod = &*lod;


	if (our_lod != NULL)
	{
		for (vector<XObjCmd8>::const_iterator cmd = our_lod->cmds.begin(); cmd != our_lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case obj8_Tris:	
				want_draw = tex_is_cockpit ? drawMode_Pan : drawMode_Tri;
				if (want_draw != drawMode) {
					if (want_draw == drawMode_Pan)	funcs->SetupPanel_f(ref);
					else							funcs->SetupPoly_f(ref);
					CHECK_GL_ERR
					drawMode = want_draw;
				}
				if (arrayMode_Tri != arrayMode)
				{
					arrayMode = arrayMode_Tri;
					glVertexPointer			(3, GL_FLOAT, 32, 				  obj.geo_tri.get(0)		  );	CHECK_GL_ERR
					glNormalPointer			(	GL_FLOAT, 32, ((const char *) obj.geo_tri.get(0)) + 12	  );	CHECK_GL_ERR
					funcs->TexCoordPointer_f(2, GL_FLOAT, 32, ((const char *) obj.geo_tri.get(0)) + 24,ref);	CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);				CHECK_GL_ERR
				glEnableClientState(GL_NORMAL_ARRAY);				CHECK_GL_ERR
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);		CHECK_GL_ERR
				glDisableClientState(GL_COLOR_ARRAY);				CHECK_GL_ERR
				glColor3f(1.0, 1.0, 1.0);							CHECK_GL_ERR
				glDrawElements(GL_TRIANGLES, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]);		CHECK_GL_ERR
				break;
			case obj8_Lines:
				if (drawMode_Lin != drawMode) {
					funcs->SetupLine_f(ref);	CHECK_GL_ERR
					drawMode = drawMode_Lin;
				}
				if (arrayMode_Lin != arrayMode)
				{
					arrayMode = arrayMode_Lin;
					glVertexPointer(3, GL_FLOAT, 24, obj.geo_lines.get(0));								CHECK_GL_ERR
					glColorPointer(3, GL_FLOAT, 24, ((const char *) obj.geo_lines.get(0)) + 12);		CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);			CHECK_GL_ERR
				glDisableClientState(GL_NORMAL_ARRAY);			CHECK_GL_ERR
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);	CHECK_GL_ERR
				glEnableClientState(GL_COLOR_ARRAY);			CHECK_GL_ERR
				glDrawElements(GL_LINES, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]);	CHECK_GL_ERR
				break;			
			case obj8_Lights:
				if (drawMode_Lgt != drawMode) {
					funcs->SetupLight_f(ref);	CHECK_GL_ERR
					drawMode = drawMode_Lgt;
				}
				if (arrayMode_Lgt != arrayMode)
				{
					arrayMode = arrayMode_Lgt;
					glVertexPointer(3, GL_FLOAT, 24, obj.geo_lights.get(0));							CHECK_GL_ERR
					glColorPointer(3, GL_FLOAT, 24, ((const char *) obj.geo_lights.get(0)) + 12);		CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);				CHECK_GL_ERR				
				glDisableClientState(GL_NORMAL_ARRAY);				CHECK_GL_ERR
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);		CHECK_GL_ERR
				glEnableClientState(GL_COLOR_ARRAY);				CHECK_GL_ERR
				glDrawArrays(GL_POINTS, cmd->idx_offset, cmd->idx_count);		CHECK_GL_ERR
				break;			
	
			case attr_Tex_Normal:	tex_is_cockpit = false;	drawMode = drawMode_Non;  break;
			case attr_Tex_Cockpit:	tex_is_cockpit = true;	break;
			case attr_No_Blend:	glDisable(GL_BLEND); CHECK_GL_ERR break;
			case attr_Blend:	glEnable(GL_BLEND);	 CHECK_GL_ERR break;

			
			case anim_Begin:	glPushMatrix();	CHECK_GL_ERR break;
			case anim_End:		glPopMatrix(); CHECK_GL_ERR break;
			case anim_Rotate:
				anim = &obj.animation[cmd->idx_offset];
				v = funcs->GetAnimParam(anim->dataref.c_str(), anim->xyzrv1[4], anim->xyzrv2[4], ref);
				v = rescale_float(anim->xyzrv1[4], anim->xyzrv2[4], anim->xyzrv1[3], anim->xyzrv2[3], v);
				glRotatef(v, anim->xyzrv1[0], anim->xyzrv1[1], anim->xyzrv1[2]);
				break;
			case anim_Translate:
				anim = &obj.animation[cmd->idx_offset];
				v = funcs->GetAnimParam(anim->dataref.c_str(), anim->xyzrv1[4], anim->xyzrv2[4], ref);
				glTranslatef(
					rescale_float(anim->xyzrv1[4], anim->xyzrv2[4], anim->xyzrv1[0], anim->xyzrv2[0], v),
					rescale_float(anim->xyzrv1[4], anim->xyzrv2[4], anim->xyzrv1[1], anim->xyzrv2[1], v),
					rescale_float(anim->xyzrv1[4], anim->xyzrv2[4], anim->xyzrv1[2], anim->xyzrv2[2], v));
				break;
	
			case attr_Shade_Flat:	glShadeModel(GL_FLAT); 	 CHECK_GL_ERR break;
			case attr_Shade_Smooth: glShadeModel(GL_SMOOTH); CHECK_GL_ERR break;
			case attr_Ambient_RGB: 	glMaterialfv(GL_FRONT,GL_AMBIENT, cmd->params); 	CHECK_GL_ERR break;
			case attr_Diffuse_RGB:  glMaterialfv(GL_FRONT,GL_DIFFUSE, cmd->params); 	CHECK_GL_ERR break;
			case attr_Emission_RGB: glMaterialfv(GL_FRONT,GL_EMISSION, cmd->params); 	CHECK_GL_ERR break;
			case attr_Specular_RGB: glMaterialfv(GL_FRONT,GL_SPECULAR, cmd->params); 	CHECK_GL_ERR break;
			case attr_Shiny_Rat:	glMaterialf (GL_FRONT,GL_SHININESS,cmd->params[0]); CHECK_GL_ERR break;
			case attr_No_Depth:		glDisable(GL_DEPTH_TEST);	CHECK_GL_ERR	break;
			case attr_Depth:		glEnable(GL_DEPTH_TEST);	CHECK_GL_ERR	break;
			case attr_Reset: { float amb[4] = { 0.2, 0.2, 0.2, 1.0 }, diff[4] = { 0.8, 0.8, 0.8, 1.0 }, zero[4] = { 0.0, 0.0, 0.0, 1.0 };
								glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
								glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
								glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
								glMaterialfv(GL_FRONT, GL_EMISSION, zero);
								glMaterialf (GL_FRONT,GL_SHININESS,0.0); } CHECK_GL_ERR break;
			case attr_Cull:		glEnable(GL_CULL_FACE);		CHECK_GL_ERR break;
			case attr_NoCull:	glDisable(GL_CULL_FACE);	CHECK_GL_ERR break;
			case attr_Offset:	if (cmd->params[0] != 0)
				{	glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(-5.0*cmd->params[0],-1.0);glDepthMask(GL_FALSE);	} else
				{	glDisable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(0.0, 0.0);glDepthMask(GL_TRUE);	} 
				CHECK_GL_ERR
				break;
			} // Case
						
		} // cmd loop
	} // our LOD
}


