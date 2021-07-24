//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================
#include "cbase.h"
#include "c_baseentity.h"
#include "hud.h"
#include "hudelement.h"
#include "clientmode.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/AnimationController.h>
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "soundenvelope.h"
#include "convar.h"
#include "hud_closecaption.h"
#include "in_buttons.h"
#ifdef MAPBASE
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_SPEAKER_NAME	256
#define MAX_COUNT_STRING	64

extern ConVar english;
extern ConVar closecaption;
class C_PointCommentaryNode;

CUtlVector< CHandle<C_PointCommentaryNode> >	g_CommentaryNodes;
bool IsInCommentaryMode( void )
{
	return (g_CommentaryNodes.Count() > 0);
}

static bool g_bTracingVsCommentaryNodes = false;

#ifdef MAPBASE
ConVar commentary_type_force( "commentary_type_force", "-1", FCVAR_NONE, "Forces all commentary nodes to use the specified type." );
ConVar commentary_type_text_endtime( "commentary_type_text_endtime", "120" );
ConVar commentary_type_image_endtime( "commentary_type_image_endtime", "120" );
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudCommentary : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCommentary, vgui::Panel );
public:
	CHudCommentary( const char *name );

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void LevelInit( void ) { g_CommentaryNodes.Purge(); }
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void StartCommentary( C_PointCommentaryNode *pNode, char *pszSpeakers, int iNode, int iNodeMax, float flStartTime, float flEndTime );
#ifdef MAPBASE
	void StartTextCommentary( C_PointCommentaryNode *pNode, const char *pszText, char *pszSpeakers, int iNode, int iNodeMax, float flStartTime, float flEndTime );
	void StartImageCommentary( C_PointCommentaryNode *pNode, const char *pszImage, char *pszSpeakers, int iNode, int iNodeMax, float flStartTime, float flEndTime );
#endif
	void StopCommentary( void );
	bool IsTheActiveNode( C_PointCommentaryNode *pNode ) { return (pNode == m_hActiveNode); }

	// vgui overrides
	virtual void Paint( void );
	virtual bool ShouldDraw( void );
#ifdef MAPBASE
	virtual void PerformLayout();
	void ResolveBounds( int width, int height );
#endif

private:
	CHandle<C_PointCommentaryNode> m_hActiveNode;
	bool	m_bShouldPaint;
	float	m_flStartTime;
	float	m_flEndTime;
	wchar_t	m_szSpeakers[MAX_SPEAKER_NAME];
	wchar_t	m_szCount[MAX_COUNT_STRING];
	CMaterialReference m_matIcon;
	bool	m_bHiding;
#ifdef MAPBASE
	int		m_iCommentaryType;
	float	m_flPanelScale;
	float	m_flOverrideX;
	float	m_flOverrideY;

	vgui::Label *m_pLabel;
	vgui::ImagePanel *m_pImage;
	vgui::HFont m_hFont;
#endif

	// Painting
	CPanelAnimationVarAliasType( int, m_iBarX, "bar_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarY, "bar_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarTall, "bar_height", "16", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarWide, "bar_width", "16", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iSpeakersX, "speaker_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iSpeakersY, "speaker_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCountXFR, "count_xpos_from_right", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCountY, "count_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconX, "icon_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconY, "icon_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconWide, "icon_width", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconTall, "icon_height", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_nIconTextureId, "icon_texture", "vgui/hud/icon_commentary", "textureid" );

#ifdef MAPBASE
	CPanelAnimationVarAliasType( int, m_iTypeAudioX, "type_audio_xpos", "190", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeAudioY, "type_audio_ypos", "350", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeAudioW, "type_audio_wide", "380", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeAudioT, "type_audio_tall", "40", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeTextX, "type_text_xpos", "180", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeTextY, "type_text_ypos", "150", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeTextW, "type_text_wide", "400", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeTextT, "type_text_tall", "200", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeTextCountXFR, "type_text_count_xpos_from_right", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTypeTextCountYFB, "type_text_count_ypos_from_bottom", "10", "proportional_int" );
	CPanelAnimationVar( Color, m_TextBackgroundColor, "BackgroundColorTextContent", "0 0 0 192" );
	CPanelAnimationVar( Color, m_TypeTextContentColor, "TextContentColor", "255 230 180 255" );
	CPanelAnimationVar( int, m_iTextBorderSpace, "type_text_border_space", "8" );
#endif

	CPanelAnimationVar( bool, m_bUseScriptBGColor, "use_script_bgcolor", "0" );
#ifdef MAPBASE
	CPanelAnimationVar( Color, m_BackgroundColor, "BackgroundColor", "Panel.BgColor" );
	CPanelAnimationVar( Color, m_ForegroundColor, "ForegroundColor", "255 170 0 255" );
#else
	CPanelAnimationVar( Color, m_BackgroundColor, "BackgroundColor", "0 0 0 0" );
#endif
	CPanelAnimationVar( Color, m_BGOverrideColor, "BackgroundOverrideColor", "Panel.BgColor" );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_PointCommentaryNode : public C_BaseAnimating
{
	DECLARE_CLASS( C_PointCommentaryNode, C_BaseAnimating );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

	virtual void OnPreDataChanged( DataUpdateType_t type );
	virtual void OnDataChanged( DataUpdateType_t type );

	void StartAudioCommentary( const char *pszCommentaryFile, C_BasePlayer *pPlayer );
#ifdef MAPBASE
	void StartTextCommentary( const char *pszCommentaryFile, C_BasePlayer *pPlayer );
	void StartImageCommentary( const char *pszCommentaryFile, C_BasePlayer *pPlayer );
#endif

	void OnRestore( void )
	{
		BaseClass::OnRestore();

		if ( m_bActive )
		{
			StopLoopingSounds();
			m_bRestartAfterRestore = true;
		}

		AddAndLockCommentaryHudGroup();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	virtual void SetDormant( bool bDormant )
	{
		if ( !IsDormant() && bDormant )
		{
			RemoveAndUnlockCommentaryHudGroup();
		}

		BaseClass::SetDormant( bDormant );
	}

	//-----------------------------------------------------------------------------
	// Cleanup
	//-----------------------------------------------------------------------------
	void UpdateOnRemove( void )
	{
		RemoveAndUnlockCommentaryHudGroup();

		StopLoopingSounds();
		BaseClass::UpdateOnRemove();
	}

	void	StopLoopingSounds( void );

	virtual bool TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace );

	void AddAndLockCommentaryHudGroup( void )
	{
		if ( !g_CommentaryNodes.Count() )
		{
			int iRenderGroup = gHUD.LookupRenderGroupIndexByName( "commentary" );
			gHUD.LockRenderGroup( iRenderGroup );
		}

		if ( g_CommentaryNodes.Find(this) == g_CommentaryNodes.InvalidIndex() )
		{
			g_CommentaryNodes.AddToTail( this );
		}
	}

	void RemoveAndUnlockCommentaryHudGroup( void )
	{
		g_CommentaryNodes.FindAndRemove( this );

		if ( !g_CommentaryNodes.Count() )
		{
			int iRenderGroup = gHUD.LookupRenderGroupIndexByName( "commentary" );
			gHUD.UnlockRenderGroup( iRenderGroup );
		}
	}

public:
	// Data received from the server
	bool		m_bActive;
	bool		m_bWasActive;
	float		m_flStartTime;
	char		m_iszCommentaryFile[MAX_PATH];
	char		m_iszCommentaryFileNoHDR[MAX_PATH];
	char		m_iszSpeakers[MAX_SPEAKER_NAME];
	int			m_iNodeNumber;
	int			m_iNodeNumberMax;
	CSoundPatch *m_sndCommentary;
	EHANDLE		m_hViewPosition;
	bool		m_bRestartAfterRestore;
#ifdef MAPBASE
	int		m_iCommentaryType;
	float	m_flPanelScale;
	float	m_flPanelX;
	float	m_flPanelY;
#endif
};

IMPLEMENT_CLIENTCLASS_DT(C_PointCommentaryNode, DT_PointCommentaryNode, CPointCommentaryNode)
	RecvPropBool( RECVINFO( m_bActive ) ),
	RecvPropTime( RECVINFO( m_flStartTime ) ),
	RecvPropString( RECVINFO(m_iszCommentaryFile) ),
	RecvPropString( RECVINFO(m_iszCommentaryFileNoHDR) ),
	RecvPropString( RECVINFO(m_iszSpeakers) ),
	RecvPropInt( RECVINFO( m_iNodeNumber ) ),
	RecvPropInt( RECVINFO( m_iNodeNumberMax ) ),
	RecvPropEHandle( RECVINFO(m_hViewPosition) ),
#ifdef MAPBASE
	RecvPropInt( RECVINFO( m_iCommentaryType ) ),
	RecvPropFloat( RECVINFO( m_flPanelScale ) ),
	RecvPropFloat( RECVINFO( m_flPanelX ) ),
	RecvPropFloat( RECVINFO( m_flPanelY ) ),
#endif
END_RECV_TABLE()

BEGIN_DATADESC( C_PointCommentaryNode )
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bWasActive, FIELD_BOOLEAN ),
	DEFINE_SOUNDPATCH( m_sndCommentary ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PointCommentaryNode::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

	m_bWasActive = m_bActive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PointCommentaryNode::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		AddAndLockCommentaryHudGroup();
	}

	if ( m_bWasActive == m_bActive && !m_bRestartAfterRestore )
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( m_bActive && pPlayer )
	{
		// Use the HDR / Non-HDR version based on whether we're running HDR or not
		char *pszCommentaryFile;
		if ( g_pMaterialSystemHardwareConfig->GetHDRType() == HDR_TYPE_NONE && m_iszCommentaryFileNoHDR && m_iszCommentaryFileNoHDR[0] )
		{
			pszCommentaryFile = m_iszCommentaryFileNoHDR;
		}
		else
		{
			pszCommentaryFile = m_iszCommentaryFile;
		}
		if ( !pszCommentaryFile || !pszCommentaryFile[0] )
		{
			engine->ServerCmd( "commentary_finishnode\n" );
			return;
		}

#ifdef MAPBASE
		int iCommentaryType = m_iCommentaryType;
		if (commentary_type_force.GetInt() != -1)
			iCommentaryType = commentary_type_force.GetInt();

		switch (iCommentaryType)
		{
			case COMMENTARY_TYPE_TEXT:
				StartTextCommentary( pszCommentaryFile, pPlayer );
				break;

			case COMMENTARY_TYPE_IMAGE:
				StartImageCommentary( pszCommentaryFile, pPlayer );
				break;

			default:
			case COMMENTARY_TYPE_AUDIO:
				StartAudioCommentary( pszCommentaryFile, pPlayer );
				break;
		}
#else
		StartAudioCommentary( pszCommentaryFile, pPlayer );
#endif
	}
	else if ( m_bWasActive )
	{
		StopLoopingSounds();

 		CHudCommentary *pHudCommentary = (CHudCommentary *)GET_HUDELEMENT( CHudCommentary );
		if ( pHudCommentary->IsTheActiveNode(this) )
		{
			pHudCommentary->StopCommentary();
		}
	}

	m_bRestartAfterRestore = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PointCommentaryNode::StartAudioCommentary( const char *pszCommentaryFile, C_BasePlayer *pPlayer )
{
	EmitSound_t es;
	es.m_nChannel = CHAN_STATIC;
	es.m_pSoundName = pszCommentaryFile;
 	es.m_SoundLevel = SNDLVL_GUNFIRE;
	es.m_nFlags = SND_SHOULDPAUSE;

	CBaseEntity *pSoundEntity;
	if ( m_hViewPosition )
	{
		pSoundEntity = m_hViewPosition;
	}
	else if ( render->GetViewEntity() )
	{
		pSoundEntity = cl_entitylist->GetEnt( render->GetViewEntity() );
		es.m_SoundLevel = SNDLVL_NONE;
	}
	else
	{
		pSoundEntity = pPlayer;
	}
	CSingleUserRecipientFilter filter( pPlayer );
	m_sndCommentary = (CSoundEnvelopeController::GetController()).SoundCreate( filter, pSoundEntity->entindex(), es );
	if ( m_sndCommentary )
	{
		(CSoundEnvelopeController::GetController()).SoundSetCloseCaptionDuration( m_sndCommentary, -1 );
		(CSoundEnvelopeController::GetController()).Play( m_sndCommentary, 1.0f, 100, m_flStartTime );
	}

	// Get the duration so we know when it finishes
	float flDuration = enginesound->GetSoundDuration( STRING( CSoundEnvelopeController::GetController().SoundGetName( m_sndCommentary ) ) ) ;

	CHudCloseCaption *pHudCloseCaption = (CHudCloseCaption *)GET_HUDELEMENT( CHudCloseCaption );
	if ( pHudCloseCaption )
	{
		// This is where we play the commentary close caption (and lock the other captions out).
		// Also, if close captions are off we force a caption in non-English
		if ( closecaption.GetBool() || ( !closecaption.GetBool() && !english.GetBool() ) )
		{
			// Clear the close caption element in preparation
			pHudCloseCaption->Reset();

			// Process the commentary caption
			pHudCloseCaption->ProcessCaptionDirect( pszCommentaryFile, flDuration );

			// Find the close caption hud element & lock it
			pHudCloseCaption->Lock();
		}
	}

	// Tell the HUD element
	CHudCommentary *pHudCommentary = (CHudCommentary *)GET_HUDELEMENT( CHudCommentary );
	pHudCommentary->StartCommentary( this, m_iszSpeakers, m_iNodeNumber, m_iNodeNumberMax, m_flStartTime, m_flStartTime + flDuration );
}

#ifdef MAPBASE
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PointCommentaryNode::StartTextCommentary( const char *pszCommentaryFile, C_BasePlayer *pPlayer )
{
	// Get the duration so we know when it finishes
	//float flDuration = enginesound->GetSoundDuration( STRING( CSoundEnvelopeController::GetController().SoundGetName( m_sndCommentary ) ) ) ;

	// TODO: Determine from text length?
	float flDuration = commentary_type_text_endtime.GetFloat();

	// Tell the HUD element
	CHudCommentary *pHudCommentary = (CHudCommentary *)GET_HUDELEMENT( CHudCommentary );
	pHudCommentary->StartTextCommentary( this, pszCommentaryFile, m_iszSpeakers, m_iNodeNumber, m_iNodeNumberMax, m_flStartTime, m_flStartTime + flDuration );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PointCommentaryNode::StartImageCommentary( const char *pszCommentaryFile, C_BasePlayer *pPlayer )
{
	// Get the duration so we know when it finishes
	//float flDuration = enginesound->GetSoundDuration( STRING( CSoundEnvelopeController::GetController().SoundGetName( m_sndCommentary ) ) ) ;

	float flDuration = commentary_type_image_endtime.GetFloat();

	// Tell the HUD element
	CHudCommentary *pHudCommentary = (CHudCommentary *)GET_HUDELEMENT( CHudCommentary );
	pHudCommentary->StartImageCommentary( this, pszCommentaryFile, m_iszSpeakers, m_iNodeNumber, m_iNodeNumberMax, m_flStartTime, m_flStartTime + flDuration );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Shut down the commentary
//-----------------------------------------------------------------------------
void C_PointCommentaryNode::StopLoopingSounds( void )
{
	if ( m_sndCommentary != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundDestroy( m_sndCommentary );
		m_sndCommentary = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: No client side trace collisions
//-----------------------------------------------------------------------------
bool C_PointCommentaryNode::TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace )
{
	if ( !g_bTracingVsCommentaryNodes )
		return false;

	return BaseClass::TestCollision( ray, mask, trace );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool IsNodeUnderCrosshair( C_BasePlayer *pPlayer )
{
	// See if the player's looking at a commentary node
	trace_t tr;
	Vector vecSrc = pPlayer->EyePosition();
	Vector vecForward;
	AngleVectors( pPlayer->EyeAngles(), &vecForward );

	g_bTracingVsCommentaryNodes = true;
	UTIL_TraceLine( vecSrc, vecSrc + vecForward * MAX_TRACE_LENGTH, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	g_bTracingVsCommentaryNodes = false;

	if ( !tr.m_pEnt )
		return false;

	return dynamic_cast<C_PointCommentaryNode*>(tr.m_pEnt);
}

//===================================================================================================================
// COMMENTARY HUD ELEMENT
//===================================================================================================================
DECLARE_HUDELEMENT( CHudCommentary );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudCommentary::CHudCommentary( const char *name ) : vgui::Panel( NULL, "HudCommentary" ), CHudElement( name )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetPaintBorderEnabled( false );
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_hActiveNode = NULL;
	m_bShouldPaint = true;

#ifdef MAPBASE
	m_pLabel = new vgui::Label( this, "HudCommentaryTextLabel", "" );
	m_pImage = new vgui::ImagePanel( this, "HudCommentaryImagePanel" );
	m_pImage->SetShouldScaleImage( true );
#endif
}

void CHudCommentary::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_bUseScriptBGColor )
	{
		SetBgColor( m_BGOverrideColor );
	}

#ifdef MAPBASE
	m_pLabel->SetPaintBackgroundType( 2 );
	m_pLabel->SetSize( 0, GetTall() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::Paint()
{
	float flDuration = (m_flEndTime - m_flStartTime);
	float flPercentage = clamp( ( gpGlobals->curtime - m_flStartTime ) / flDuration, 0.f, 1.f );

	if ( !m_hActiveNode )
	{
		if ( !m_bHiding )
		{
			m_bHiding = true;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HideCommentary" );

			CHudCloseCaption *pHudCloseCaption = (CHudCloseCaption *)GET_HUDELEMENT( CHudCloseCaption );
			if ( pHudCloseCaption )
			{
				pHudCloseCaption->Reset();
			}
		}
	}
	else
	{
		// Detect the end of the commentary
		if ( flPercentage >= 1 && m_hActiveNode )
		{
			m_hActiveNode = NULL;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HideCommentary" );

			engine->ServerCmd( "commentary_finishnode\n" );
		}
	}

	if ( !m_bShouldPaint )
		return;

	int x, y, wide, tall;
	GetBounds( x, y, wide, tall );

	int xOffset = m_iBarX;
	int yOffset = m_iBarY;

	// Find our fade based on our time shown
	Color clr = m_ForegroundColor;

#ifdef MAPBASE
	switch (m_iCommentaryType)
	{
		case COMMENTARY_TYPE_TEXT:
			{
				// Figure out the size before setting bounds
				int lW, lT;
				m_pLabel->GetContentSize( lW, lT );

				lT += (m_iTextBorderSpace * 2);

				vgui::surface()->DrawSetColor( clr );
				vgui::surface()->DrawOutlinedRect( xOffset, yOffset, xOffset + (m_iBarWide * m_flPanelScale), yOffset + (lT /** m_flPanelScale*/) ); //m_iTypeTextT - (yOffset /*+ m_iBarTall*/) );
			} break;

		case COMMENTARY_TYPE_IMAGE:
			{
				// Figure out the size before setting bounds
				int iW, iT;
				m_pImage->GetSize( iW, iT );
				//vgui::surface()->DrawGetTextureSize( m_pImage->GetImage()->GetID(), iW, iT );

				iW += (m_iTextBorderSpace * 2);
				iT += (m_iTextBorderSpace * 2);

				vgui::surface()->DrawSetColor( clr );
				vgui::surface()->DrawOutlinedRect( xOffset, yOffset, xOffset + iW, yOffset + iT ); //m_iTypeTextT - (yOffset /*+ m_iBarTall*/) );
			} break;

		default:
		case COMMENTARY_TYPE_AUDIO:
			{
				// Draw the progress bar
				vgui::surface()->DrawSetColor( clr );
				vgui::surface()->DrawOutlinedRect( xOffset, yOffset, xOffset+m_iBarWide, yOffset+m_iBarTall );
				vgui::surface()->DrawSetColor( clr );
				vgui::surface()->DrawFilledRect( xOffset+2, yOffset+2, xOffset+(int)(flPercentage*m_iBarWide)-2, yOffset+m_iBarTall-2 );
			} break;
	}
#else
	// Draw the progress bar
	vgui::surface()->DrawSetColor( clr );
	vgui::surface()->DrawOutlinedRect( xOffset, yOffset, xOffset+m_iBarWide, yOffset+m_iBarTall );
	vgui::surface()->DrawSetColor( clr );
	vgui::surface()->DrawFilledRect( xOffset+2, yOffset+2, xOffset+(int)(flPercentage*m_iBarWide)-2, yOffset+m_iBarTall-2 );
#endif

	// Draw the speaker names
	vgui::surface()->DrawSetTextFont( m_hFont );
	vgui::surface()->DrawSetTextColor( clr ); 
	vgui::surface()->DrawSetTextPos( m_iSpeakersX, m_iSpeakersY );
	vgui::surface()->DrawPrintText( m_szSpeakers, wcslen(m_szSpeakers) );

	if ( COMMENTARY_BUTTONS & IN_ATTACK )
	{
		int iY = m_iBarY + m_iBarTall + YRES(4);
		wchar_t wzFinal[512] = L"";

		wchar_t *pszText = g_pVGuiLocalize->Find( "#Commentary_PrimaryAttack" );
		if ( pszText )
		{
			UTIL_ReplaceKeyBindings( pszText, 0, wzFinal, sizeof( wzFinal ) );
			vgui::surface()->DrawSetTextPos( m_iSpeakersX, iY );
			vgui::surface()->DrawPrintText( wzFinal, wcslen(wzFinal) );
		}

		pszText = g_pVGuiLocalize->Find( "#Commentary_SecondaryAttack" );
		if ( pszText )
		{
			int w, h;
			UTIL_ReplaceKeyBindings( pszText, 0, wzFinal, sizeof( wzFinal ) );
			vgui::surface()->GetTextSize( m_hFont, wzFinal, w, h );
			vgui::surface()->DrawSetTextPos( m_iBarX + m_iBarWide - w, iY );
			vgui::surface()->DrawPrintText( wzFinal, wcslen(wzFinal) );
		}
	}

	// Draw the commentary count
	// Determine our text size, and move that far in from the right hand size (plus the offset)
	int iCountWide, iCountTall;
	vgui::surface()->GetTextSize( m_hFont, m_szCount, iCountWide, iCountTall );

#ifdef MAPBASE
	if (m_iCommentaryType != COMMENTARY_TYPE_AUDIO)
		vgui::surface()->DrawSetTextPos( wide - m_iTypeTextCountXFR - iCountWide, tall - m_iTypeTextCountYFB - iCountTall );
	else
#endif
	vgui::surface()->DrawSetTextPos( wide - m_iCountXFR - iCountWide, m_iCountY );

	vgui::surface()->DrawPrintText( m_szCount, wcslen( m_szCount ) );

	// Draw the icon
 	vgui::surface()->DrawSetColor( Color(255,170,0,GetAlpha()) );
	vgui::surface()->DrawSetTexture(m_nIconTextureId);
	vgui::surface()->DrawTexturedRect( m_iIconX, m_iIconY, m_iIconWide, m_iIconTall );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudCommentary::ShouldDraw()
{
	return ( m_hActiveNode || GetAlpha() > 0 );
}

#ifdef MAPBASE
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::PerformLayout()
{
	BaseClass::PerformLayout();

	switch (m_iCommentaryType)
	{
		case COMMENTARY_TYPE_TEXT:
			{
				int xOffset = m_iBarX;
				int yOffset = m_iBarY;

				int x, y, wide, tall;
				GetBounds( x, y, wide, tall );

				// Figure out the size before setting bounds
				int lW, lT;
				m_pLabel->GetContentSize( lW, lT );

				lW = (float)(m_iBarWide * m_flPanelScale) - m_iTextBorderSpace;
				//lT = (float)lT * m_flPanelScale; // Don't affect height when scaling

				m_pLabel->SetBounds(
					xOffset + m_iTextBorderSpace,
					yOffset + m_iTextBorderSpace,
					lW, lT );

				lW += (float)((m_iTextBorderSpace * 2) + (xOffset * 2));
				lT += (float)((m_iTextBorderSpace * 2) + (yOffset * 2));

				ResolveBounds( lW, lT );
			} break;

		case COMMENTARY_TYPE_IMAGE:
			{
				int xOffset = m_iBarX;
				int yOffset = m_iBarY;

				// Figure out the size before setting bounds
				int iW, iT;
				//m_pImage->GetImage()->GetSize( iW, iT );
				vgui::surface()->DrawGetTextureSize( m_pImage->GetImage()->GetID(), iW, iT );
				if (iW <= 0)
					iW = 1;

				int iTargetSize = (m_iBarWide - m_iTextBorderSpace);
				iT *= (iTargetSize / iW);
				iW = iTargetSize;

				iW = (float)iW * m_flPanelScale;
				iT = (float)iT * m_flPanelScale;

				m_pImage->SetBounds(
					xOffset + m_iTextBorderSpace,
					yOffset + m_iTextBorderSpace,
					iW, iT );

				iW += (float)((m_iTextBorderSpace * 2) + (xOffset * 2));
				iT += (float)((m_iTextBorderSpace * 2) + (yOffset * 2));

				ResolveBounds( iW, iT );
			} break;

		default:
		case COMMENTARY_TYPE_AUDIO:
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Resolves position on screen; Heavily borrows from CHudMessage::XPosition/YPosition
//-----------------------------------------------------------------------------
void CHudCommentary::ResolveBounds( int width, int height )
{
	int xPos;
	int yPos;

	// ====== X ======
	if ( m_flOverrideX == -1 )
	{
		xPos = (ScreenWidth() - width) * 0.5f;
	}
	else
	{
		if ( m_flOverrideX < 0 )
			xPos = (1.0 + m_flOverrideX) * ScreenWidth() - width;	// Align to right
		else
			xPos = m_flOverrideX * (ScreenWidth() - width);
	}

	// Clamp to edge of screen
	if ( xPos + width > ScreenWidth() )
		xPos = ScreenWidth() - width;
	else if ( xPos < 0 )
		xPos = 0;

	// ====== Y ======
	if ( m_flOverrideY == -1 )
	{
		yPos = (ScreenHeight() - height) * 0.5f;
	}
	else
	{
		if ( m_flOverrideY < 0 )
			yPos = (1.0 + m_flOverrideY) * ScreenHeight() - height;	// Align to bottom
		else
			yPos = m_flOverrideY * (ScreenHeight() - height);
	}

	// Clamp to edge of screen
	if ( yPos + height > ScreenHeight() )
		yPos = ScreenHeight() - height;
	else if ( yPos < 0 )
		yPos = 0;

	SetBounds( xPos, yPos, width, height );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::Init( void )
{ 
	m_matIcon.Init( "vgui/hud/icon_commentary", TEXTURE_GROUP_VGUI );

#ifdef MAPBASE
	SetProportional( true );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::VidInit( void )
{ 
	SetAlpha(0);
	StopCommentary();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::StartCommentary( C_PointCommentaryNode *pNode, char *pszSpeakers, int iNode, int iNodeMax, float flStartTime, float flEndTime )
{
	if ( (flEndTime - flStartTime) <= 0 )
		return;

	m_hActiveNode = pNode;
	m_flStartTime = flStartTime;
	m_flEndTime = flEndTime;
	m_bHiding = false;
#ifdef MAPBASE
	m_iCommentaryType = COMMENTARY_TYPE_AUDIO;
	m_flPanelScale = pNode->m_flPanelScale;
	m_flOverrideX = pNode->m_flPanelX;
	m_flOverrideY = pNode->m_flPanelY;
#endif
	g_pVGuiLocalize->ConvertANSIToUnicode( pszSpeakers, m_szSpeakers, sizeof( m_szSpeakers ) );

#ifdef MAPBASE
	SetBounds( m_iTypeAudioX, m_iTypeAudioY, m_iTypeAudioW, m_iTypeAudioT );
	SetBgColor( m_bUseScriptBGColor ? m_BGOverrideColor : m_BackgroundColor );

	m_pLabel->SetPaintEnabled( false );
	m_pImage->SetPaintEnabled( false );
	m_pImage->EvictImage();

	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	m_hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "CommentaryDefault" );
	if ( !m_hFont )
	{
		m_hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "Default" );
	}
#endif

	// Don't draw the element itself if closecaptions are on (and captions are always on in non-english mode)
	ConVarRef pCVar( "closecaption" );
	if ( pCVar.IsValid() )
	{
		m_bShouldPaint = ( !pCVar.GetBool() && english.GetBool() );
	}
	else
	{
		m_bShouldPaint = true;
	}
	SetPaintBackgroundEnabled( m_bShouldPaint );

	char sz[MAX_COUNT_STRING];
	Q_snprintf( sz, sizeof(sz), "%d \\ %d", iNode, iNodeMax );
	g_pVGuiLocalize->ConvertANSIToUnicode( sz, m_szCount, sizeof(m_szCount) );

	// If the commentary just started, play the commentary fade in.
	if ( fabs(flStartTime - gpGlobals->curtime) < 1.0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ShowCommentary" );
	}
	else
	{
		// We're reloading a savegame that has an active commentary going in it. Don't fade in.
		SetAlpha( 255 );
	}
}

#ifdef MAPBASE
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::StartTextCommentary( C_PointCommentaryNode *pNode, const char *pszText, char *pszSpeakers, int iNode, int iNodeMax, float flStartTime, float flEndTime )
{
	if ( (flEndTime - flStartTime) <= 0 )
		return;

	m_hActiveNode = pNode;
	m_flStartTime = flStartTime;
	m_flEndTime = flEndTime;
	m_bHiding = false;
	m_iCommentaryType = COMMENTARY_TYPE_TEXT;
	m_flPanelScale = pNode->m_flPanelScale;
	m_flOverrideX = pNode->m_flPanelX;
	m_flOverrideY = pNode->m_flPanelY;
	g_pVGuiLocalize->ConvertANSIToUnicode( pszSpeakers, m_szSpeakers, sizeof( m_szSpeakers ) );

	SetBounds( m_iTypeTextX, m_iTypeTextY, m_iTypeTextW, m_iTypeTextT );
	SetBgColor( m_bUseScriptBGColor ? m_BGOverrideColor : m_TextBackgroundColor );

	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	m_hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "CommentaryDefault" );
	if ( !m_hFont )
	{
		m_hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "Default" );
	}

	m_pLabel->SetText( pszText );
	m_pLabel->SetFont( m_hFont );
	m_pLabel->SetWrap( true );
	m_pLabel->SetPaintEnabled( true );
	m_pLabel->SetPaintBackgroundEnabled( false );
	m_pLabel->SetPaintBorderEnabled( false );
	//m_pLabel->SizeToContents();
	m_pLabel->SetContentAlignment( vgui::Label::a_northwest );
	m_pLabel->SetFgColor( m_TypeTextContentColor );

	m_pImage->SetPaintEnabled( false );
	m_pImage->EvictImage();

	m_bShouldPaint = true;
	SetPaintBackgroundEnabled( m_bShouldPaint );

	char sz[MAX_COUNT_STRING];
	Q_snprintf( sz, sizeof(sz), "%d \\ %d", iNode, iNodeMax );
	g_pVGuiLocalize->ConvertANSIToUnicode( sz, m_szCount, sizeof(m_szCount) );

	// If the commentary just started, play the commentary fade in.
	if ( fabs(flStartTime - gpGlobals->curtime) < 1.0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ShowCommentary" );
	}
	else
	{
		// We're reloading a savegame that has an active commentary going in it. Don't fade in.
		SetAlpha( 255 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::StartImageCommentary( C_PointCommentaryNode *pNode, const char *pszImage, char *pszSpeakers, int iNode, int iNodeMax, float flStartTime, float flEndTime )
{
	if ( (flEndTime - flStartTime) <= 0 )
		return;

	m_hActiveNode = pNode;
	m_flStartTime = flStartTime;
	m_flEndTime = flEndTime;
	m_bHiding = false;
	m_iCommentaryType = COMMENTARY_TYPE_IMAGE;
	m_flPanelScale = pNode->m_flPanelScale;
	m_flOverrideX = pNode->m_flPanelX;
	m_flOverrideY = pNode->m_flPanelY;
	g_pVGuiLocalize->ConvertANSIToUnicode( pszSpeakers, m_szSpeakers, sizeof( m_szSpeakers ) );
	
	SetBounds( m_iTypeTextX, m_iTypeTextY, m_iTypeTextW, m_iTypeTextT );
	SetBgColor( m_bUseScriptBGColor ? m_BGOverrideColor : m_TextBackgroundColor );

	m_pLabel->SetPaintEnabled( false );

	m_pImage->SetPaintEnabled( true );
	m_pImage->SetImage( pszImage );
	m_pImage->SetWide( m_iBarWide - m_iTextBorderSpace );

	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	m_hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "CommentaryDefault" );
	if ( !m_hFont )
	{
		m_hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "Default" );
	}

	m_bShouldPaint = true;
	SetPaintBackgroundEnabled( m_bShouldPaint );

	char sz[MAX_COUNT_STRING];
	Q_snprintf( sz, sizeof(sz), "%d \\ %d", iNode, iNodeMax );
	g_pVGuiLocalize->ConvertANSIToUnicode( sz, m_szCount, sizeof(m_szCount) );

	// If the commentary just started, play the commentary fade in.
	if ( fabs(flStartTime - gpGlobals->curtime) < 1.0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ShowCommentary" );
	}
	else
	{
		// We're reloading a savegame that has an active commentary going in it. Don't fade in.
		SetAlpha( 255 );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCommentary::StopCommentary( void )
{
	m_hActiveNode = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CommentaryModeShouldSwallowInput( C_BasePlayer *pPlayer )
{
	if ( !IsInCommentaryMode() )	
		return false;

	if ( pPlayer->m_nButtons & COMMENTARY_BUTTONS )
	{
		// Always steal the secondary attack
		if ( pPlayer->m_nButtons & IN_ATTACK2 )
			return true;

		// See if there's any nodes ahead of us.
		if ( IsNodeUnderCrosshair( pPlayer ) )
			return true;
	}

	return false;
}