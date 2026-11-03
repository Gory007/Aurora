# ---------------------------------------------------------------
# poc-radio.spec — ручной RPM-спецификатор для Aurora build-engine.
#
# Почему этот файл нужен:
#   Если в rpm/ есть *.spec, sfdk использует ЕГО и не пытается
#   сгенерировать spec из yaml. Сгенерированный же spec по умолчанию
#   содержит секцию %build со стандартным вызовом ./configure,
#   которого в qmake-проекте нет -> "No such file or directory".
#
# Здесь %build явно описан через qmake-qt5 + make, а %files перечисляет
# то, что реально попадает в пакет.
#
# ВАЖНО: символы "%" здесь легальны — это RPM-файл, а не YAML.
# (В yaml-версии рецепта % писать нельзя.)
# ---------------------------------------------------------------

%define debug_package %{nil}
%undefine __brp_mangle_shebangs

Name:           poc-radio
Version:        0.1
Release:        1
Summary:        Proof-of-concept push-to-talk radio client for Aurora OS
License:        MIT
Group:          Applications/Comms And Sync
URL:            https://example.org/poc-radio
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:  qt5-qtbase-devel
Requires:       qt5-qtbase

%description
Консольный клиент рации: подключение к серверу по TCP, вход в канал,
PTT (нажал - говорю, отпустил - слушаю). Учебный проект, этапы 1-2.

%prep
%setup -q -n %{name}

%build
qmake-qt5 %{name}.pro
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}%{_bindir}
install -m 0755 %{name} %{buildroot}%{_bindir}/%{name}

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}

%changelog
