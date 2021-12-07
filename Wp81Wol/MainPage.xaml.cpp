//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Wol.h"
#include <cvt/wstring>
#include <codecvt>

using namespace Wp81Wol;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
	InitializeComponent();
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

	// TODO: Prepare page for display here.

	// TODO: If your application contains multiple pages, ensure that you are
	// handling the hardware Back button by registering for the
	// Windows::Phone::UI::Input::HardwareButtons.BackPressed event.
	// If you are using the NavigationHelper provided by some templates,
	// this event is handled for you.

	// Data source.
	Platform::Collections::Vector<String^>^ itemsList =
		ref new Platform::Collections::Vector<String^>();
	itemsList->Append("Item A");
	itemsList->Append("Item B");

	listView1->ItemsSource = itemsList;
}

void Wp81Wol::Debug(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[100];
	vsnprintf_s(buffer, sizeof(buffer), format, args);

	OutputDebugStringA(buffer);

	va_end(args);
}

void Wp81Wol::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Debug("Button was clicked!\n");

	greetingOutput->Text = "Hello, " + nameInput->Text + "!";

	((Platform::Collections::Vector<String^>^)listView1->ItemsSource)->Append(nameInput->Text);

	unsigned port{ 60000 };
	unsigned long bcast{ 0xFFFFFFFF };
	Wol::send_wol("D0:50:99:4B:CB:0D", port, bcast);

}


void Wp81Wol::MainPage::ItemView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	String^ item = (String^)e->ClickedItem;

	std::string utf8 = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(item->Data());

	Debug("Item clicked ! %s\n", utf8.c_str());
}
