//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Wol.h"
#include "ComputerItem.h"
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
using namespace Windows::Phone::UI::Input;
using namespace Windows::Storage;
using namespace concurrency;
using namespace Windows::Networking::Connectivity;

#define filename "wp81WolFile.txt"
#define fileVersion "v1"

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

// List of computers
Platform::Collections::Vector<Wp81Wol::ComputerItem^>^ listComputerItems = nullptr;

// Index (of the list) of the edited computer
int editedComputerItemIndex = -1;

// Local storage folder.
Windows::Storage::StorageFolder^ localFolder = nullptr;

// Timer to auto-hide the "notification Flyout".
Windows::UI::Xaml::DispatcherTimer^ timer = nullptr;

MainPage::MainPage()
{
	InitializeComponent();

	HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs ^>(this, &MainPage::HardwareButtons_BackPressed);

	localFolder = ApplicationData::Current->LocalFolder;
}

void MainPage::HardwareButtons_BackPressed(Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs^ e)
{
	// Return to the list of computer.
	if (!IsStateListComputerItem())
	{
		//Indicate the back button press is handled so the app does not exit
		e->Handled = true;

		StateComputerItemList();
	}

	ReadComputerItemListFile();
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


	listComputerItems = ref new Platform::Collections::Vector<Wp81Wol::ComputerItem^>();
	ListViewComputerItem->ItemsSource = listComputerItems;

	// Init the list of computers from a previously created file.
	ReadComputerItemListFile();
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

/**
* see https://social.msdn.microsoft.com/Forums/sqlserver/en-US/2fda9c75-135c-4ead-9a6c-28d78a83b6e0/force-winsock2-socket-to-use-wifi-connection?forum=wpdevelop
*/
String^ getWiFiIP()
{
	std::vector<String^> ipAddresses;
	auto hostnames = NetworkInformation::GetHostNames();

	for (unsigned int i = 0; i < hostnames->Size; ++i)
	{
		auto hn = hostnames->GetAt(i);

		//IanaInterfaceType == 71 => Wifi
		//IanaInterfaceType == 6 => Ethernet (Emulator)
		if (hn->IPInformation != nullptr)
		{
			auto type = hn->IPInformation->NetworkAdapter->IanaInterfaceType;
			if (type == 71 || type == 6)
			{
				auto ipAddress = hn->DisplayName;
				ipAddresses.push_back(ipAddress);
			}
		}
	}

	if (ipAddresses.size() < 1)
	{
		return nullptr;
	}
	else
	{
		return ipAddresses[ipAddresses.size() - 1];
	}
}



void Wp81Wol::MainPage::ItemView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	ComputerItem^ item = (ComputerItem^)e->ClickedItem;

	std::string macAddress = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(item->getMacAddress()->Data());
	Debug("Item clicked ! %s\n", macAddress.c_str());

	//Getting WiFi Interface IP
	Platform::String^ wifiIp = getWiFiIP();
	Debug("Wifi interface IP:"); OutputDebugString(wifiIp->Data()); Debug("\n");
	std::string wifiIpstr = "";
	if (wifiIp != nullptr) {
		wifiIpstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wifiIp->Data());
	}

	unsigned port{ 60000 };
	unsigned long bcast{ 0xFFFFFFFF };
	String^ notificationText = L"WOL packet sent.";
	Windows::UI::Xaml::Style^ style = safe_cast<Windows::UI::Xaml::Style ^>(Resources->Lookup("InfoNotificationStyle"));
	try
	{
		Wol::send_wol(macAddress.c_str(), port, bcast, wifiIpstr);
	}
	catch (const std::runtime_error& error)
	{
		Debug("Wol error: %s\n", error.what());
		std::string s_str = std::string(error.what());
		std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
		const wchar_t* w_char = wid_str.c_str();
		notificationText = ref new String(w_char);
		style = safe_cast<Windows::UI::Xaml::Style ^>(Resources->Lookup("ErrorNotificationStyle"));
	}
	FlyoutNotificationText->Text = notificationText;
	FlyoutNotification->FlyoutPresenterStyle = safe_cast<Windows::UI::Xaml::Style ^>(style);
	FlyoutNotification->ShowAt((ListView^)sender);

	timer = ref new Windows::UI::Xaml::DispatcherTimer();
	TimeSpan ts;
	ts.Duration = 20000000; // 100-ns
	timer->Interval = ts;
	timer->Start();
	auto registrationtoken = timer->Tick += ref new EventHandler<Object^>(this, &MainPage::OnTick);
}

void MainPage::OnTick(Object^ sender, Object^ e) {
	timer->Stop();
	FlyoutNotification->Hide();
}

void Wp81Wol::MainPage::AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ b = (Button^)sender;
	if (b->Tag->ToString() == "Add")
	{
		// Want to add a new computer
		TextBoxName->Text = "";
		TextBoxMacAddress->Text = "";
		StateCreateComputerItem();
	}
	else if (b->Tag->ToString() == "Save")
	{
		if (IsStateEditComputerItem())
		{ 
			// Save existing computer.
			ComputerItem^ item = listComputerItems->GetAt(editedComputerItemIndex);
			item->setName(TextBoxName->Text);
			item->setMacAddress(TextBoxMacAddress->Text);
			// Force a VectorChanged event.
			listComputerItems->SetAt(editedComputerItemIndex, item);
		}
		else
		{
			// Save/create a new ComputerItem
			ComputerItem^ item = ref new ComputerItem(TextBoxName->Text, TextBoxMacAddress->Text);
			listComputerItems->Append(item);
		}
		WriteComputerItemListFile();
		StateComputerItemList();
	}
	else if (b->Tag->ToString() == "Delete")
	{
		// Delete a computer
		listComputerItems->RemoveAt(editedComputerItemIndex);
		WriteComputerItemListFile();
		StateComputerItemList();
	}	
	else if (b->Tag->ToString() == "Help")
	{
		// Want to see the help page.
		StateHelp();
	}
}

void Wp81Wol::MainPage::ItemView_Holding(Platform::Object^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs^ e)
{
	// Hold and release a computer item.
	if (e->HoldingState == Windows::UI::Input::HoldingState::Completed)
	{
		StackPanel^sp = (StackPanel^)sender;
		ComputerItem^ item = (ComputerItem^)sp->DataContext;
		TextBoxName->Text = item->getName();
		TextBoxMacAddress->Text = item->getMacAddress();

		unsigned int index;
		if (listComputerItems->IndexOf(item, &index))
		{
			// Item found
			editedComputerItemIndex = index;
			StateEditComputerItem();
		}
		else
		{
			// Item not found ?
			Debug("Item not found");
		}
	}
}


////////////////////////////////////

void Wp81Wol::MainPage::StateComputerItemList()
{
	PanelListComputerItem->Visibility = Windows::UI::Xaml::Visibility::Visible;
	PanelEditComputerItem->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	PanelHelp->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	AddAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	SaveAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	DeleteAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	HelpAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	editedComputerItemIndex = -1;
	if (listComputerItems->Size == 0)
	{
		ListViewIsEmpty->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
	else
	{
		ListViewIsEmpty->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	}
}

void Wp81Wol::MainPage::StateCreateComputerItem()
{
	PanelListComputerItem->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	PanelEditComputerItem->Visibility = Windows::UI::Xaml::Visibility::Visible;
	PanelHelp->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	AddAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	SaveAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	DeleteAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	HelpAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void Wp81Wol::MainPage::StateEditComputerItem()
{
	PanelListComputerItem->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	PanelEditComputerItem->Visibility = Windows::UI::Xaml::Visibility::Visible;
	PanelHelp->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	AddAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	SaveAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	DeleteAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	HelpAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void Wp81Wol::MainPage::StateHelp()
{
	PanelListComputerItem->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	PanelEditComputerItem->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	PanelHelp->Visibility = Windows::UI::Xaml::Visibility::Visible;
	AddAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	SaveAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	DeleteAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	HelpAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

bool Wp81Wol::MainPage::IsStateEditComputerItem()
{
	return editedComputerItemIndex != -1;
}

bool Wp81Wol::MainPage::IsStateListComputerItem()
{
	return PanelListComputerItem->Visibility == Windows::UI::Xaml::Visibility::Visible;
}

////////////////////////////////////

void Wp81Wol::MainPage::ReadComputerItemListFile()
{
	listComputerItems->Clear();

	create_task(localFolder->GetFileAsync(filename)).then([this](StorageFile^ file)
	{
		return FileIO::ReadLinesAsync(file);
	}).then([this](IVector<String^>^ lines)
	{
		String^ version = lines->GetAt(0);
		std::string utf8 = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(version->Data());
		Debug("File version %s\n", utf8.c_str());

		for (unsigned int i = 1; i < lines->Size; i += 2)
		{
			ComputerItem^ item = ref new ComputerItem(lines->GetAt(i), lines->GetAt(i + 1));
			listComputerItems->Append(item);
		}
	}).then([this](task<void> t)
	{

		try
		{
			t.get();
			// .get() didn' t throw, so we succeeded.
			Debug("Read file succeeded.\n");
		}
		catch (Platform::COMException^ e)
		{
			//The system cannot find the specified file.
			OutputDebugString(e->Message->Data());
			// First time that this application is launched?
			// Create empty file
			WriteComputerItemListFile();
		}

	});
}

void Wp81Wol::MainPage::WriteComputerItemListFile()
{
	create_task(localFolder->CreateFileAsync(filename, CreationCollisionOption::ReplaceExisting)).then(
		[this](StorageFile^ file)
	{
		IVector<String^>^ lines = ref new Platform::Collections::Vector<String^>();
		lines->Append(fileVersion);
		for (unsigned int i = 0; i < listComputerItems->Size; i++)
		{
			ComputerItem^ item = listComputerItems->GetAt(i);
			lines->Append(item->getName());
			lines->Append(item->getMacAddress());
		}
		return FileIO::AppendLinesAsync(file, lines);
	});
}


