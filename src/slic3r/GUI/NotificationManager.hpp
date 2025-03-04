#ifndef slic3r_GUI_NotificationManager_hpp_
#define slic3r_GUI_NotificationManager_hpp_

#include "GUI_App.hpp"
#include "Plater.hpp"
#include "GLCanvas3D.hpp"
#include "Event.hpp"
#include "I18N.hpp"

#include <libslic3r/ObjectID.hpp>
#include <libslic3r/Technologies.hpp>

#include <wx/time.h>

#include <string>
#include <vector>
#include <deque>
#include <unordered_set>

namespace Slic3r {
namespace GUI {

using EjectDriveNotificationClickedEvent = SimpleEvent;
wxDECLARE_EVENT(EVT_EJECT_DRIVE_NOTIFICAION_CLICKED, EjectDriveNotificationClickedEvent);
using ExportGcodeNotificationClickedEvent = SimpleEvent;
wxDECLARE_EVENT(EVT_EXPORT_GCODE_NOTIFICAION_CLICKED, ExportGcodeNotificationClickedEvent);
using PresetUpdateAvailableClickedEvent = SimpleEvent;
wxDECLARE_EVENT(EVT_PRESET_UPDATE_AVAILABLE_CLICKED, PresetUpdateAvailableClickedEvent);

class GLCanvas3D;
class ImGuiWrapper;
enum class InfoItemType;

enum class NotificationType
{
	CustomNotification,
	// Notification on end of slicing and G-code processing (the full G-code preview is available).
	// Contains a hyperlink to export the G-code to a removable media.
	SlicingComplete,
//	SlicingNotPossible,
	// Notification on end of export to a removable media, with hyperling to eject the external media.
	// Obsolete by ExportFinished
//	ExportToRemovableFinished,
	// Notification on end of export, with hyperling to see folder and eject if export was to external media.
	// Own subclass.
	ExportFinished,
	// Works on OSX only.
	//FIXME Do we want to have it on Linux and Windows? Is it possible to get the Disconnect event on Windows?
	Mouse3dDisconnected,
//	Mouse3dConnected,
//	NewPresetsAviable,
	// Notification on the start of PrusaSlicer, when a new PrusaSlicer version is published.
	// Contains a hyperlink to open a web browser pointing to the PrusaSlicer download location.
	NewAppAvailable,
	// Notification on the start of PrusaSlicer, when updates of system profiles are detected.
	// Contains a hyperlink to execute installation of the new system profiles.
	PresetUpdateAvailable,
//	LoadingFailed,
	// Errors emmited by Print::validate
	// difference from Slicing error is that they disappear not grey out at update_background_process
	ValidateError,
	// Notification emitted by Print::validate
	ValidateWarning,
	// Slicing error produced by BackgroundSlicingProcess::validate() or by the BackgroundSlicingProcess background
	// thread thowing a SlicingError exception.
	SlicingError,
	// Slicing warnings, issued by the slicing process.
	// Slicing warnings are registered for a particular Print milestone or a PrintObject and its milestone.
	SlicingWarning,
	// Object partially outside the print volume. Cannot slice.
	PlaterError,
	// Object fully outside the print volume, or extrusion outside the print volume. Slicing is not disabled.
	PlaterWarning,
	// Progress bar instead of text.
	ProgressBar,
	// Progress bar with info from Print Host Upload Queue dialog.
	PrintHostUpload,
	// Notification, when Color Change G-code is empty and user try to add color change on DoubleSlider.
    EmptyColorChangeCode,
    // Notification that custom supports/seams were deleted after mesh repair.
    CustomSupportsAndSeamRemovedAfterRepair,
    // Notification that auto adding of color changes is impossible
    EmptyAutoColorChange,
    // Notification about detected sign
    SignDetected,
    // Notification telling user to quit SLA supports manual editing
    QuitSLAManualMode,
    // Desktop integration basic info
    DesktopIntegrationSuccess,
    DesktopIntegrationFail,
    UndoDesktopIntegrationSuccess,
    UndoDesktopIntegrationFail,
    // Notification that a printer has more extruders than are supported by MM Gizmo/segmentation.
    MmSegmentationExceededExtrudersLimit,
	// Did you know Notification appearing on startup with arrows to change hint
	DidYouKnowHint,
	// Shows when  ObjectList::update_info_items finds information that should be stressed to the user
	// Might contain logo taken from gizmos
	UpdatedItemsInfo,
	// Give user advice to simplify object with big amount of triangles
	SimplifySuggestion
};

class NotificationManager
{
public:
	enum class NotificationLevel : int
	{
		// The notifications will be presented in the order of importance, thus these enum values
		// are sorted by the importance.
		// "Good to know" notification, usually but not always with a quick fade-out.
		RegularNotification = 1,
		// Information notification without a fade-out or with a longer fade-out.
		ImportantNotification,
		// Important notification with progress bar, no fade-out, might appear again after closing.
		ProgressBarNotification,
		// Warning, no fade-out.
		WarningNotification,
		// Error, no fade-out.
		ErrorNotification,
	};

	NotificationManager(wxEvtHandler* evt_handler);
	
	// Push a prefabricated notification from basic_notifications (see the table at the end of this file).
	void push_notification(const NotificationType type, int timestamp = 0);
	// Push a NotificationType::CustomNotification with NotificationLevel::RegularNotification and 10s fade out interval.
	void push_notification(const std::string& text, int timestamp = 0);
	// Push a NotificationType::CustomNotification with provided notification level and 10s for RegularNotification.
	// ErrorNotification and ImportantNotification are never faded out.
    void push_notification(NotificationType type, NotificationLevel level, const std::string& text, const std::string& hypertext = "",
                           std::function<bool(wxEvtHandler*)> callback = std::function<bool(wxEvtHandler*)>(), int timestamp = 0);
	// Creates Validate Error notification with a custom text and no fade out.
	void push_validate_error_notification(const std::string& text);
	// Creates Slicing Error notification with a custom text and no fade out.
	void push_slicing_error_notification(const std::string& text);
	// Creates Slicing Warning notification with a custom text and no fade out.
	void push_slicing_warning_notification(const std::string& text, bool gray, ObjectID oid, int warning_step);
	// marks slicing errors as gray
	void set_all_slicing_errors_gray(bool g);
	// marks slicing warings as gray
	void set_all_slicing_warnings_gray(bool g);
//	void set_slicing_warning_gray(const std::string& text, bool g);
	// immediately stops showing slicing errors
	void close_slicing_errors_and_warnings();
	void close_slicing_error_notification(const std::string& text);
	// Release those slicing warnings, which refer to an ObjectID, which is not in the list.
	// living_oids is expected to be sorted.
	void remove_slicing_warnings_of_released_objects(const std::vector<ObjectID>& living_oids);
	// Object partially outside of the printer working space, cannot print. No fade out.
	void push_plater_error_notification(const std::string& text);
	// Object fully out of the printer working space and such. No fade out.
	void push_plater_warning_notification(const std::string& text);
	// Closes error or warning of the same text
	void close_plater_error_notification(const std::string& text);
	void close_plater_warning_notification(const std::string& text);
	// Creates special notification slicing complete.
	// If large = true (Plater side bar is closed), then printing time and export button is shown
	// at the notification and fade-out is disabled. Otherwise the fade out time is set to 10s.
	void push_slicing_complete_notification(int timestamp, bool large);
	// Add a print time estimate to an existing SlicingComplete notification.
	void set_slicing_complete_print_time(const std::string &info);
	// Called when the side bar changes its visibility, as the "slicing complete" notification supplements
	// the "slicing info" normally shown at the side bar.
	void set_slicing_complete_large(bool large);
	// Exporting finished, show this information with path, button to open containing folder and if ejectable - eject button
	void push_exporting_finished_notification(const std::string& path, const std::string& dir_path, bool on_removable);
	// notification with progress bar
	void push_upload_job_notification(int id, float filesize, const std::string& filename, const std::string& host, float percentage = 0);
	void set_upload_job_notification_percentage(int id, const std::string& filename, const std::string& host, float percentage);
	void upload_job_notification_show_canceled(int id, const std::string& filename, const std::string& host);
	void upload_job_notification_show_error(int id, const std::string& filename, const std::string& host);
	// Hint (did you know) notification
	void push_hint_notification(bool open_next);
	bool is_hint_notification_open();
	void push_updated_item_info_notification(InfoItemType type);
	// Close old notification ExportFinished.
	void new_export_began(bool on_removable);
	// finds ExportFinished notification and closes it if it was to removable device
	void device_ejected();
	// renders notifications in queue and deletes expired ones
	void render_notifications(GLCanvas3D& canvas, float overlay_width);
	// finds and closes all notifications of given type
	void close_notification_of_type(const NotificationType type);
	// Hides warnings in G-code preview. Should be called from plater only when 3d view/ preview is changed
    void set_in_preview(bool preview);
	// Calls set_in_preview to apply appearing or disappearing of some notificatons;
	void apply_in_preview() { set_in_preview(m_in_preview); }
	// Move to left to avoid colision with variable layer height gizmo.
	void set_move_from_overlay(bool move) { m_move_from_overlay = move; }
	// perform update_state on each notification and ask for more frames if needed, return true for render needed
	bool update_notifications(GLCanvas3D& canvas);
	// returns number of all notifications shown
	size_t get_notification_count() const;
private:
	// duration 0 means not disapearing
	struct NotificationData {
		NotificationType         type;
		NotificationLevel        level;
		// Fade out time
		const int                duration;
		const std::string        text1;
		const std::string        hypertext;
		// Callback for hypertext - returns true if notification should close after triggering
		// Usually sends event to UI thread thru wxEvtHandler.
		// Examples in basic_notifications.
        std::function<bool(wxEvtHandler*)> callback;
		const std::string        text2;
	};

	// Cache of IDs to identify and reuse ImGUI windows.
	class NotificationIDProvider
	{
	public:
		int 		allocate_id();
		void 		release_id(int id);

	private:
		// Next ID used for naming the ImGUI windows.
		int       			m_next_id{ 1 };
		// IDs of ImGUI windows, which were released and they are ready for reuse.
		std::vector<int>	m_released_ids;
	};

	//Pop notification - shows only once to user.
	class PopNotification
	{
	public:

		enum class EState
		{
			Unknown,		  // NOT initialized
			Hidden,
			Shown,			  // Requesting Render at some time if duration != 0
			NotFading,		  // Never jumps to state Fading out even if duration says so
			FadingOut,        // Requesting Render at some time
			ClosePending,     // Requesting Render
			Finished,         // Requesting Render
			Hovered,		  // Followed by Shown 
			Paused
		};

		PopNotification(const NotificationData &n, NotificationIDProvider &id_provider, wxEvtHandler* evt_handler);
		virtual ~PopNotification() { if (m_id) m_id_provider.release_id(m_id); }
		virtual void           render(GLCanvas3D& canvas, float initial_y, bool move_from_overlay, float overlay_width);
		// close will dissapear notification on next render
		virtual void           close() { m_state = EState::ClosePending; wxGetApp().plater()->get_current_canvas3D()->schedule_extra_frame(0);}
		// data from newer notification of same type
		void                   update(const NotificationData& n);
		bool                   is_finished() const { return m_state == EState::ClosePending || m_state == EState::Finished; }
		// returns top after movement
		float                  get_top() const { return m_top_y; }
		//returns top in actual frame
		float                  get_current_top() const { return m_top_y; }
		const NotificationType get_type() const { return m_data.type; }
		const NotificationData& get_data() const { return m_data; }
		const bool             is_gray() const { return m_is_gray; }
		void                   set_gray(bool g) { m_is_gray = g; }
		virtual bool           compare_text(const std::string& text) const;
        void                   hide(bool h) { if (is_finished()) return; m_state = h ? EState::Hidden : EState::Unknown; }
		// sets m_next_render with time of next mandatory rendering. Delta is time since last render.
		bool                   update_state(bool paused, const int64_t delta);
		int64_t 		       next_render() const { return is_finished() ? 0 : m_next_render; }
		EState                 get_state()  const { return m_state; }
		bool				   is_hovered() const { return m_state == EState::Hovered; } 
		void				   set_hovered() { if (m_state != EState::Finished && m_state != EState::ClosePending && m_state != EState::Hidden && m_state != EState::Unknown) m_state = EState::Hovered; }
		// set start of notification to now. Used by delayed notifications
		void                   reset_timer() { m_notification_start = GLCanvas3D::timestamp_now(); m_state = EState::Shown; }
	protected:
		// Call after every size change
		virtual void init();
		// Calculetes correct size but not se it in imgui!
		virtual void set_next_window_size(ImGuiWrapper& imgui);
		virtual void render_text(ImGuiWrapper& imgui,
			                     const float win_size_x, const float win_size_y,
			                     const float win_pos_x , const float win_pos_y);
		virtual void render_close_button(ImGuiWrapper& imgui,
			                             const float win_size_x, const float win_size_y,
			                             const float win_pos_x , const float win_pos_y);
		virtual void render_hypertext(ImGuiWrapper& imgui,
			                          const float text_x, const float text_y,
		                              const std::string text,
		                              bool more = false);
		// Left sign could be error or warning sign
		virtual void render_left_sign(ImGuiWrapper& imgui);
		virtual void render_minimize_button(ImGuiWrapper& imgui,
			                                const float win_pos_x, const float win_pos_y);
		// Hypertext action, returns true if notification should close.
		// Action is stored in NotificationData::callback as std::function<bool(wxEvtHandler*)>
		virtual bool on_text_click();
	
		// Part of init(), counts horizontal spacing like left indentation 
		virtual void count_spaces();
		// Part of init(), counts end lines
		virtual void count_lines();
		// returns true if PopStyleColor should be called later to pop this push
		virtual bool push_background_color();

		const NotificationData m_data;
		// For reusing ImGUI windows.
		NotificationIDProvider &m_id_provider;
		int              m_id{ 0 };

		// State for rendering
		EState           m_state                { EState::Unknown };

		// Time values for rendering fade-out

		int64_t		 	 m_fading_start{ 0LL };

		// first appereance of notification or last hover;
		int64_t		 	 m_notification_start;
		// time to next must-do render
		int64_t          m_next_render{ std::numeric_limits<int64_t>::max() };
		float            m_current_fade_opacity{ 1.0f };

		// Notification data

		// Main text
		std::string      m_text1;
		// Clickable text
		std::string      m_hypertext;
		// Aditional text after hypertext - currently not used
		std::string      m_text2;

		// inner variables to position notification window, texts and buttons correctly

		// all space without text
		float            m_window_width_offset;
		// Space on left side without text
		float            m_left_indentation;
		// Total size of notification window - varies based on monitor
		float            m_window_height        { 56.0f };  
		float            m_window_width         { 450.0f };
		//Distance from bottom of notifications to top of this notification
		float            m_top_y                { 0.0f };  
		// Height of text - Used as basic scaling unit!
		float            m_line_height;
		// endlines for text1, hypertext excluded
        std::vector<size_t> m_endlines;
		// endlines for text2
		std::vector<size_t> m_endlines2;
		// Gray are f.e. eorrors when its uknown if they are still valid
		bool             m_is_gray              { false };
		//if multiline = true, notification is showing all lines(>2)
		bool             m_multiline            { false };
		// True if minimized button is rendered, helps to decide where is area for invisible close button
		bool             m_minimize_b_visible   { false };
        size_t           m_lines_count{ 1 };
	    // Target for wxWidgets events sent by clicking on the hyperlink available at some notifications.
		wxEvtHandler*    m_evt_handler;
	};

	class SlicingCompleteLargeNotification : public PopNotification
	{
	public:
		SlicingCompleteLargeNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler, bool largeds);
		void			set_large(bool l);
		bool			get_large() { return m_is_large; }
		void			set_print_info(const std::string &info);
		void			render(GLCanvas3D& canvas, float initial_y, bool move_from_overlay, float overlay_width) override
		{
			// This notification is always hidden if !large (means side bar is collapsed)
			if (!get_large() && !is_finished()) 
				m_state = EState::Hidden;
			PopNotification::render(canvas, initial_y, move_from_overlay, overlay_width);
		}
	protected:
		void render_text(ImGuiWrapper& imgui,
			                     const float win_size_x, const float win_size_y,
			                     const float win_pos_x, const float win_pos_y) 
			                     override;
		bool        m_is_large;
		bool        m_has_print_info { false };
        std::string m_print_info;
	};

	class SlicingWarningNotification : public PopNotification
	{
	public:
		SlicingWarningNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler) : PopNotification(n, id_provider, evt_handler) {}
		ObjectID 	object_id;
		int    		warning_step;
	};

	class PlaterWarningNotification : public PopNotification
	{
	public:
		PlaterWarningNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler) : PopNotification(n, id_provider, evt_handler) {}
		void	     close()  override { if(is_finished()) return; m_state = EState::Hidden; wxGetApp().plater()->get_current_canvas3D()->schedule_extra_frame(0); }
		void		 real_close()      { m_state = EState::ClosePending; wxGetApp().plater()->get_current_canvas3D()->schedule_extra_frame(0); }
		void         show()            { m_state = EState::Unknown; }
	};

	
	class ProgressBarNotification : public PopNotification
	{
	public:
		
		ProgressBarNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler, float percentage) : PopNotification(n, id_provider, evt_handler) { }
		virtual void set_percentage(float percent) { m_percentage = percent; }
	protected:
		virtual void init() override;
		virtual void count_lines() override;
		
		virtual void	render_text(ImGuiWrapper& imgui,
									const float win_size_x, const float win_size_y,
									const float win_pos_x, const float win_pos_y) override;
		virtual void	render_bar(ImGuiWrapper& imgui,
									const float win_size_x, const float win_size_y,
									const float win_pos_x, const float win_pos_y) ;
		virtual void	render_cancel_button(ImGuiWrapper& imgui,
									const float win_size_x, const float win_size_y,
									const float win_pos_x, const float win_pos_y)
		{}
		void			render_minimize_button(ImGuiWrapper& imgui,
			const float win_pos_x, const float win_pos_y) override {}
		float				m_percentage;
		
		bool				m_has_cancel_button {false};
		// local time of last hover for showing tooltip
		
	};

	

	class PrintHostUploadNotification : public ProgressBarNotification
	{
	public:
		enum class UploadJobState
		{
			PB_PROGRESS,
			PB_ERROR,
			PB_CANCELLED,
			PB_COMPLETED
		};
		PrintHostUploadNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler, float percentage, int job_id, float filesize)
			:ProgressBarNotification(n, id_provider, evt_handler, percentage)
			, m_job_id(job_id)
			, m_file_size(filesize)
		{
			m_has_cancel_button = true;
			set_percentage(percentage);
		}
		static std::string	get_upload_job_text(int id, const std::string& filename, const std::string& host) { return /*"[" + std::to_string(id) + "] " + */filename + " -> " + host; }
		void				set_percentage(float percent) override;
		void				cancel() { m_uj_state = UploadJobState::PB_CANCELLED; m_has_cancel_button = false; }
		void				error()  { m_uj_state = UploadJobState::PB_ERROR;     m_has_cancel_button = false; init(); }
		bool				compare_job_id(const int other_id) const { return m_job_id == other_id; }
		bool				compare_text(const std::string& text) const override { return false; }
	protected:
		void        init() override;
		void		count_spaces() override;
		bool		push_background_color() override;
		void		render_bar(ImGuiWrapper& imgui,
								const float win_size_x, const float win_size_y,
								const float win_pos_x, const float win_pos_y) override;
		void		render_cancel_button(ImGuiWrapper& imgui,
											const float win_size_x, const float win_size_y,
											const float win_pos_x, const float win_pos_y) override;
		void		render_left_sign(ImGuiWrapper& imgui) override;
		// Identifies job in cancel callback
		int					m_job_id;
		// Size of uploaded size to be displayed in MB
		float			    m_file_size;
		long				m_hover_time{ 0 };
		UploadJobState	m_uj_state{ UploadJobState::PB_PROGRESS };
	};

	class ExportFinishedNotification : public PopNotification
	{
	public:
		ExportFinishedNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler, bool to_removable,const std::string& export_path,const std::string& export_dir_path)
			: PopNotification(n, id_provider, evt_handler)
			, m_to_removable(to_removable)
			, m_export_path(export_path)
			, m_export_dir_path(export_dir_path)
		    {
				m_multiline = true;
			}
		bool        m_to_removable;
		std::string m_export_path;
		std::string m_export_dir_path;
	protected:
		// Reserves space on right for more buttons
		void count_spaces() override;
		void render_text(ImGuiWrapper& imgui,
						 const float win_size_x, const float win_size_y,
						 const float win_pos_x, const float win_pos_y) override;
		// Renders also button to open directory with exported path and eject removable media
		void render_close_button(ImGuiWrapper& imgui,
								 const float win_size_x, const float win_size_y,
								 const float win_pos_x, const float win_pos_y) override;
		void         render_eject_button(ImGuiWrapper& imgui,
			                             const float win_size_x, const float win_size_y,
			                             const float win_pos_x, const float win_pos_y);
		void render_minimize_button(ImGuiWrapper& imgui, const float win_pos_x, const float win_pos_y) override
			{ m_minimize_b_visible = false; }
		bool on_text_click() override;
		// local time of last hover for showing tooltip
		long      m_hover_time { 0 };
	};

	class UpdatedItemsInfoNotification : public PopNotification
	{
	public:
		UpdatedItemsInfoNotification(const NotificationData& n, NotificationIDProvider& id_provider, wxEvtHandler* evt_handler, InfoItemType info_item_type)
			: PopNotification(n, id_provider, evt_handler)
		{
			//m_types_and_counts.emplace_back(info_item_type, 1);
		}
		void count_spaces() override;
		void add_type(InfoItemType type);
	protected:
		void render_left_sign(ImGuiWrapper& imgui) override;
		std::vector<std::pair<InfoItemType, size_t>> m_types_and_counts;
	};

	// in HintNotification.hpp
	class HintNotification;

	// Data of waiting notifications
	struct DelayedNotification
	{
		std::unique_ptr<PopNotification>	notification;
		std::function<bool(void)>           condition_callback;
		int64_t								remaining_time;
		int64_t                             delay_interval;
		
		DelayedNotification(std::unique_ptr<PopNotification> n, std::function<bool(void)> cb, int64_t r, int64_t d)
		: notification(std::move(n))
	    , condition_callback(cb)
		, remaining_time(r)
		, delay_interval(d)
		{}
	};

	//pushes notification into the queue of notifications that are rendered
	//can be used to create custom notification
	bool push_notification_data(const NotificationData& notification_data, int timestamp);
	bool push_notification_data(std::unique_ptr<NotificationManager::PopNotification> notification, int timestamp);
	// Delayed notifications goes first to the m_waiting_notifications vector and only after remaining time is <= 0
	// and condition callback is success, notification is regular pushed from update function.
	// Otherwise another delay interval waiting. Timestamp is 0. 
	// Note that notification object is constructed when being added to the waiting list, but there are no updates called on it and its timer is reset at regular push.
	// Also note that no control of same notification is done during push_delayed_notification but if waiting notif fails to push, it continues waiting.
	void push_delayed_notification(std::unique_ptr<NotificationManager::PopNotification> notification, std::function<bool(void)> condition_callback, int64_t initial_delay, int64_t delay_interval);
	// Removes all notifications of type from m_waiting_notifications
	void stop_delayed_notifications_of_type(const NotificationType type);
	//finds older notification of same type and moves it to the end of queue. returns true if found
	bool activate_existing(const NotificationManager::PopNotification* notification);
	// Put the more important notifications to the bottom of the list.
	void sort_notifications();
	// If there is some error notification active, then the "Export G-code" notification after the slicing is finished is suppressed.
    bool has_slicing_error_notification();
    
	// Target for wxWidgets events sent by clicking on the hyperlink available at some notifications.
	wxEvtHandler*                m_evt_handler;
	// Cache of IDs to identify and reuse ImGUI windows.
	NotificationIDProvider 		 m_id_provider;
	std::deque<std::unique_ptr<PopNotification>> m_pop_notifications;
	// delayed waiting notifications, first is remaining time
	std::deque<DelayedNotification> m_waiting_notifications;
	//timestamps used for slicing finished - notification could be gone so it needs to be stored here
	std::unordered_set<int>      m_used_timestamps;
	// True if G-code preview is active. False if the Plater is active.
	bool                         m_in_preview { false };
	// True if the layer editing is enabled in Plater, so that the notifications are shifted left of it.
	bool                         m_move_from_overlay { false };
	// Timestamp of last rendering
	int64_t						 m_last_render { 0LL };
	// Notification types that can be shown multiple types at once (compared by text)
	const std::vector<NotificationType> m_multiple_types = { 
		NotificationType::CustomNotification, 
		NotificationType::PlaterWarning, 
		NotificationType::ProgressBar, 
		NotificationType::PrintHostUpload, 
        NotificationType::SimplifySuggestion
	};
	//prepared (basic) notifications
	static const NotificationData basic_notifications[];
};

}//namespace GUI
}//namespace Slic3r

#endif //slic3r_GUI_NotificationManager_hpp_
