/* vim:set et sts=4: */
#include "engine.h"
#include <skv8.h>

typedef struct _IBusAvroEngine IBusAvroEngine;
typedef struct _IBusAvroEngineClass IBusAvroEngineClass;

struct _IBusAvroEngine {
	IBusEngine parent;

    /* members */
    GString *preedit;
    GString *bnpreedit;
    gint cursor_pos;
    gint candidate_pos;

    IBusLookupTable *table;
};

struct _IBusAvroEngineClass {
	IBusEngineClass parent;
};

/* functions prototype */
static void	ibus_avro_engine_class_init	(IBusAvroEngineClass	*klass);
static void	ibus_avro_engine_init		(IBusAvroEngine		*engine);
static void	ibus_avro_engine_destroy		(IBusAvroEngine		*engine);
static gboolean 
			ibus_avro_engine_process_key_event
                                            (IBusEngine             *engine,
                                             guint               	 keyval,
                                             guint               	 keycode,
                                             guint               	 modifiers);
static void 
            ibus_avro_engine_candidate_clicked
                                            (IBusEngine             *engine,
                                             guint                   index,
                                             guint                   button,
                                             guint                   state);
static void ibus_avro_engine_focus_in    (IBusEngine             *engine);
static void ibus_avro_engine_focus_out   (IBusEngine             *engine);
static void ibus_avro_engine_reset       (IBusEngine             *engine);
static void ibus_avro_engine_enable      (IBusEngine             *engine);
static void ibus_avro_engine_disable     (IBusEngine             *engine);
static void ibus_engine_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_avro_engine_set_capabilities
                                            (IBusEngine             *engine,
                                             guint                   caps);
static void ibus_avro_engine_page_up     (IBusEngine             *engine);
static void ibus_avro_engine_page_down   (IBusEngine             *engine);
static void ibus_avro_engine_cursor_up   (IBusEngine             *engine);
static void ibus_avro_engine_cursor_down (IBusEngine             *engine);
static void ibus_avro_property_activate  (IBusEngine             *engine,
                                             const gchar            *prop_name,
                                             gint                    prop_state);
static void ibus_avro_engine_property_show
											(IBusEngine             *engine,
                                             const gchar            *prop_name);
static void ibus_avro_engine_property_hide
											(IBusEngine             *engine,
                                             const gchar            *prop_name);

static void ibus_avro_engine_commit_string
                                            (IBusAvroEngine      *avro,
                                             const gchar            *string);
static void ibus_avro_engine_update      (IBusAvroEngine      *avro);
static void ibus_avro_engine_update_preedit (IBusAvroEngine *avro);

G_DEFINE_TYPE (IBusAvroEngine, ibus_avro_engine, IBUS_TYPE_ENGINE)

static void
ibus_avro_engine_class_init (IBusAvroEngineClass *klass)
{
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
	
	ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_avro_engine_destroy;

    engine_class->process_key_event = ibus_avro_engine_process_key_event;
    engine_class->candidate_clicked = ibus_avro_engine_candidate_clicked;
}

static void
ibus_avro_engine_init (IBusAvroEngine *avro)
{
    loadjs();
    avro->preedit = g_string_new ("");
    avro->bnpreedit = g_string_new ("");
    avro->cursor_pos = 0;
    avro->candidate_pos = 0;

    avro->table = ibus_lookup_table_new (9, 0, TRUE, TRUE);
    g_object_ref_sink (avro->table);
}

static void
ibus_avro_engine_destroy (IBusAvroEngine *avro)
{
    if (avro->preedit) {
        g_string_free (avro->bnpreedit, TRUE);
        avro->bnpreedit = NULL;
        g_string_free (avro->preedit, TRUE);
        avro->preedit = NULL;
    }

    if (avro->table) {
        g_object_unref (avro->table);
        avro->table = NULL;
    }

	((IBusObjectClass *) ibus_avro_engine_parent_class)->destroy ((IBusObject *)avro);
}

static void
ibus_avro_engine_update_lookup_table (IBusAvroEngine *avro)
{
    gchar ** sugs;
    gint n_sug, i;
    gboolean retval;

    //gchar *sk[2] = {"sarim","khan"};



    ibus_engine_update_auxiliary_text((IBusEngine *) avro,ibus_text_new_from_string(avro->preedit->str),TRUE);

    if (avro->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) avro);
       return;
    }

    ibus_lookup_table_clear (avro->table);

    std::vector<std::string> suggts = recvlists( avro->preedit->str);
    n_sug = suggts.size();


    if (/*sugs == NULL ||*/ n_sug == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) avro);
        return;
    }



    for (i = 0; i < n_sug; i++) {
        ibus_lookup_table_append_candidate (avro->table, ibus_text_new_from_string (suggts[i].c_str()));
    }

    ibus_engine_update_lookup_table ((IBusEngine *) avro, avro->table, TRUE);

    avro->candidate_pos = 0;

    ibus_avro_engine_update_preedit (avro);

    //if (sugs)
    //    avro_dict_free_suggestions (dict, sugs);
}

static void
ibus_avro_engine_update_preedit (IBusAvroEngine *avro)
{
    IBusText *text;
    gint retval;

    if (avro->preedit->len > 0)
    g_string_assign (avro->bnpreedit,  ibus_text_get_text(ibus_lookup_table_get_candidate(avro->table,avro->candidate_pos)));


    text = ibus_text_new_from_static_string (avro->bnpreedit->str);
    text->attrs = ibus_attr_list_new ();
    
    ibus_attr_list_append (text->attrs,
                           ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, avro->bnpreedit->len));

/*
    if (avro->preedit->len > 0) {
        retval = avro_dict_check (dict, avro->preedit->str, avro->preedit->len);
        if (retval != 0) {
            ibus_attr_list_append (text->attrs,
                               ibus_attr_foreground_new (0xff0000, 0, avro->preedit->len));
        }
    }
 */   
    ibus_engine_update_preedit_text ((IBusEngine *)avro,
                                     text,
                                     avro->cursor_pos,
                                     TRUE);

}

/* commit preedit to client and update preedit */
static gboolean
ibus_avro_engine_commit_preedit (IBusAvroEngine *avro)
{
    if (avro->preedit->len == 0)
        return FALSE;
    
    ibus_avro_engine_commit_string (avro, avro->bnpreedit->str);
    g_string_assign (avro->preedit, "");
    g_string_assign (avro->bnpreedit, "");
    avro->cursor_pos = 0;

    ibus_avro_engine_update (avro);

    return TRUE;
}


static void
ibus_avro_engine_commit_string (IBusAvroEngine *avro,
                                   const gchar       *string)
{
    IBusText *text;
    text = ibus_text_new_from_static_string (string);
    ibus_engine_commit_text ((IBusEngine *)avro, text);
}

static void
ibus_avro_engine_update (IBusAvroEngine *avro)
{
    ibus_avro_engine_update_lookup_table (avro);
    if (avro->preedit->len == 0){
        g_string_assign (avro->preedit, "");
        g_string_assign (avro->bnpreedit, "");
        avro->cursor_pos = 0;
        ibus_engine_hide_auxiliary_text((IBusEngine *)avro);
        ibus_avro_engine_update_preedit (avro);
    }
    //ibus_avro_engine_update_preedit (avro);
    //ibus_engine_hide_lookup_table ((IBusEngine *)avro);

}

static void ibus_avro_engine_candidate_pos_updated(IBusAvroEngine *avro)
{
    ibus_lookup_table_set_cursor_pos(avro->table,avro->candidate_pos);
    ibus_engine_update_lookup_table ((IBusEngine *) avro, avro->table, TRUE);
    ibus_avro_engine_update_preedit (avro);
}

static void ibus_avro_engine_candidate_clicked
                                            (IBusEngine             *engine,
                                             guint                   index,
                                             guint                   button,
                                             guint                   state)
{
    
    IBusAvroEngine *avro = (IBusAvroEngine *)engine;

    if (avro->preedit->len > 0){
        avro->candidate_pos = index;
        ibus_avro_engine_candidate_pos_updated(avro);

    }
}

#define is_alpha(c) (  ( (c) >= 33 && (c) <= 126) || ((c) >= 0xffb0 && (c) <= 0xffb9) || ((c) >= 0xffab) || ((c) >= 0xffae ) || ((c) >= 0xffaf ) || ((c) >= 0xffaa) || ((c) >= 0xffab)|| ((c) >= 0xffac)|| ((c) >= 0xffad)|| ((c) >= 0xffae)|| ((c) >= 0xffaf) )

static gboolean 
ibus_avro_engine_process_key_event (IBusEngine *engine,
                                       guint       keyval,
                                       guint       keycode,
                                       guint       modifiers)
{

    IBusText *text;
    IBusAvroEngine *avro = (IBusAvroEngine *)engine;

    if (modifiers & IBUS_RELEASE_MASK)
        return FALSE;

    modifiers &= (IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

    //if (modifiers == IBUS_CONTROL_MASK && keyval == IBUS_s) {
    //    ibus_avro_engine_update_lookup_table (avro);
    //    return TRUE;
    //}

    if (modifiers != 0) {
        if (avro->preedit->len == 0)
            return FALSE;
        else
            return TRUE;
    }


    switch (keyval) {

    case IBUS_space:
        //g_string_append (avro->preedit, " ");
        ibus_avro_engine_commit_preedit (avro);
        return FALSE;
    case IBUS_Return:
        ibus_avro_engine_commit_preedit (avro);
        return FALSE;

    case IBUS_Tab:
        ibus_avro_engine_commit_preedit (avro);
        return FALSE;

    case IBUS_Escape:
        if (avro->preedit->len == 0)
            return FALSE;

        ibus_avro_engine_commit_preedit (avro);
        return FALSE;        

    case IBUS_Left:
        if (avro->preedit->len == 0)
            return FALSE;
        if (avro->cursor_pos > 0) {
            avro->cursor_pos --;
            ibus_avro_engine_update (avro);
        }
        return TRUE;

    case IBUS_Right:
        if (avro->preedit->len == 0)
            return FALSE;
        if (avro->cursor_pos < avro->preedit->len) {
            avro->cursor_pos ++;
            ibus_avro_engine_update (avro);
        }
        return TRUE;
    
    case IBUS_Up:
        if (avro->preedit->len == 0)
            return FALSE;
        // if (avro->cursor_pos != 0) {
        //     avro->cursor_pos = 0;
        //     ibus_avro_engine_update (avro);
        // }
        if (avro->candidate_pos > 0){
            ibus_lookup_table_cursor_up(avro->table);
            avro->candidate_pos--;
            ibus_avro_engine_candidate_pos_updated(avro);
        }
        return TRUE;

    case IBUS_Down:
        if (avro->preedit->len == 0)
            return FALSE;
        
        // if (avro->cursor_pos != avro->preedit->len) {
        //     avro->cursor_pos = avro->preedit->len;
        //     ibus_avro_engine_update (avro);
        // }
        if (avro->candidate_pos < 8){
            ibus_lookup_table_cursor_down(avro->table);
            avro->candidate_pos++;
            ibus_avro_engine_candidate_pos_updated(avro);
        }
        
        return TRUE;
    
    case IBUS_BackSpace:
        if (avro->preedit->len == 0)
            return FALSE;
        if (avro->cursor_pos > 0) {
            avro->cursor_pos --;
            g_string_erase (avro->preedit, avro->cursor_pos, 1);
            ibus_avro_engine_update (avro);
        }
        return TRUE;
    
    case IBUS_Delete:
        if (avro->preedit->len == 0)
            return FALSE;
        if (avro->cursor_pos < avro->preedit->len) {
            g_string_erase (avro->preedit, avro->cursor_pos, 1);
            ibus_avro_engine_update (avro);
        }
        return TRUE;
    }

    if (is_alpha (keyval)) {
        g_string_insert_c (avro->preedit,
                           avro->cursor_pos,
                           keyval);

        avro->cursor_pos ++;
        ibus_avro_engine_update (avro);
        return TRUE;
    }

    return FALSE;
}

