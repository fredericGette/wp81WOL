#pragma once

namespace Wp81Wol
{
	[Windows::UI::Xaml::Data::Bindable]
	public ref class ComputerItem sealed
	{

	public:

		ComputerItem(Platform::String^ Name, Platform::String^ MacAddress);
		Platform::String^ getName();
		void setName(Platform::String ^ Name);
		Platform::String^ getMacAddress();
		void setMacAddress(Platform::String^ MacAddress);

		property Platform::String^ Name {
			Platform::String^ get() { return this->name; }
			void set(Platform::String^ Name) { this->name = Name; }
		}
		property Platform::String^ MacAddress {
			Platform::String^ get() { return this->macAddress; }
			void set(Platform::String^ MacAddress) { this->macAddress = MacAddress; }
		}

	private:

		Platform::String^ name;
		Platform::String^ macAddress;

	};
}