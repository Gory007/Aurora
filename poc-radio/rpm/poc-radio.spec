# ---------------------------------------------------------------
# poc-radio.spec - hand-written RPM spec for the Aurora build-engine.
#
# STATUS: FALLBACK ONLY. The normal workflow uses rpm/poc-radio.yaml and
# lets spectacle generate the spec from it (answer "Y" when sfdk asks
# whether to overwrite a non-spectacle spec). Keep this file around as a
# reference of what the generated spec must contain for a qmake project;
# if you decide to build from this spec instead, delete/rename the yaml
# or keep both in sync.
#
# WHY THE BUILD STAGE IS WRITTEN OUT BY HAND
#   The default generated spec calls ./configure, which a qmake project
#   does not have -> "Bad exit status" in the build stage. Here the build
#   stage is explicitly qmake + make.
#
# TWO TRAPS THAT BROKE EARLIER BUILD ATTEMPTS
#   1) Encoding: this file must stay pure ASCII (no Cyrillic). Saved as
#      UTF-8 with Russian comments, rpmbuild read it with the wrong
#      codepage and failed with
#      "Name field must be present in package ... can't parse".
#   2) RPM directives inside comments: rpmbuild expands them anyway and
#      warns "Macro expanded in comment". So no percent tokens are used
#      in the prose above.
# ---------------------------------------------------------------

%define debug_package %{nil}

Name:           poc-radio
Version:        0.1
Release:        1
Summary:        Proof-of-concept push-to-talk radio client for Aurora OS
License:        MIT
Group:          Applications/Publishing And Printing
URL:            https://example.org/poc-radio
Source0:        %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core) pkgconfig(Qt5Network)
Requires:       qt5-qtbase

%description
Console push-to-talk radio client prototype for Aurora OS: TCP
signalling connection, channel join, PTT talk and listen.
Training project, stages 1-2.

%prep
%setup -q -n %{name}

%build
%qmake_qt5 %{name}.pro
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}%{_bindir}
install -m 0755 %{name} %{buildroot}%{_bindir}/%{name}

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}

%changelog
