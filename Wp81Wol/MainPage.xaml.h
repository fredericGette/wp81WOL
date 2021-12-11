//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "ComputerItem.h"

namespace Wp81Wol
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		void HardwareButtons_BackPressed(Object ^ sender, Windows::Phone::UI::Input::BackPressedEventArgs ^ e);

	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		void OnTick(Object ^ sender, Object ^ e);
	private:
		void ItemView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void StateComputerItemList();
		void StateCreateComputerItem();
		void StateEditComputerItem();
		void StateHelp();
		bool IsStateEditComputerItem();
		bool IsStateListComputerItem();
		void ReadComputerItemListFile();
		void WriteComputerItemListFile();
		void ItemView_Holding(Platform::Object^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs^ e);
	};

	void Debug(const char * format, ...);

}
