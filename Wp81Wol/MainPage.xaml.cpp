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

#define filename "wp81WolFile.txt"

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

Platform::Collections::Vector<Wp81Wol::ComputerItem^>^ listComputerItems = nullptr;
int editedComputerItemIndex = -1;
Windows::Storage::StorageFolder^ localFolder = nullptr;

MainPage::MainPage()
{
	InitializeComponent();

	HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs ^>(this, &MainPage::HardwareButtons_BackPressed);

	localFolder = ApplicationData::Current->LocalFolder;
}

void MainPage::HardwareButtons_BackPressed(Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs^ e)
{
	Debug("Back pressed\n");

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

	/*create_task(localFolder->CreateFileAsync(filename, CreationCollisionOption::ReplaceExisting)).then(
		[this](StorageFile^ file)
	{
		return FileIO::WriteTextAsync(file, "v1\nORDI TELE\nD0:50:99:4B:CB:0D\nORDI-FRED\nAB:CB:19:1B:1B:1D");
	});*/
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


void Wp81Wol::MainPage::ItemView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	ComputerItem^ item = (ComputerItem^)e->ClickedItem;

	std::string utf8 = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(item->getMacAddress()->Data());

	Debug("Item clicked ! %s\n", utf8.c_str());

	unsigned port{ 60000 };
	unsigned long bcast{ 0xFFFFFFFF };
	try
	{
		Wol::send_wol(utf8.c_str(), port, bcast);
	}
	catch (const std::runtime_error& error)
	{
		Debug("Wol error: %s", error.what());
	}
}

void Wp81Wol::MainPage::AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ b = (Button^)sender;
	if (b->Tag->ToString() == "Add")
	{
		TextBoxName->Text = "";
		TextBoxMacAddress->Text = "";
		StateCreateComputerItem();
	}
	else if (b->Tag->ToString() == "Save")
	{
		if (IsStateEditComputerItem())
		{ 
			ComputerItem^ item = listComputerItems->GetAt(editedComputerItemIndex);
			item->setName(TextBoxName->Text);
			item->setMacAddress(TextBoxMacAddress->Text);
			// Force a VectorChanged event.
			listComputerItems->SetAt(editedComputerItemIndex, item);
		}
		else
		{
			// Create a new ComputerItem
			ComputerItem^ item = ref new ComputerItem(TextBoxName->Text, TextBoxMacAddress->Text);
			listComputerItems->Append(item);
		}
		WriteComputerItemListFile();
		StateComputerItemList();
	}
	else if (b->Tag->ToString() == "Delete")
	{
		listComputerItems->RemoveAt(editedComputerItemIndex);
		WriteComputerItemListFile();
		StateComputerItemList();
	}	
	else if (b->Tag->ToString() == "Help")
	{
		StateHelp();
	}
}

void Wp81Wol::MainPage::ItemView_Holding(Platform::Object^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs^ e)
{
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

/////////

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
		lines->Append("v1");
		for (unsigned int i = 0; i < listComputerItems->Size; i++)
		{
			ComputerItem^ item = listComputerItems->GetAt(i);
			lines->Append(item->getName());
			lines->Append(item->getMacAddress());
		}
		FileIO::AppendLinesAsync(file, lines);
	});
}