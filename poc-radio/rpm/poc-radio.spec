# ---------------------------------------------------------------
# poc-radio.spec - hand-written RPM spec for the Aurora build-engine.
#
# WHY THIS FILE EXISTS
#   If rpm/ contains a spec file, sfdk uses it as-is and does NOT
#   generate one from the yaml recipe. The auto-generated spec put a
#   plain configure call into the build stage, which a qmake project
#   does not have -> "Bad exit status ... (build stage)".
#   Here the build stage is explicitly qmake + make.
#
# TWO TRAPS THAT BROKE EARLIER BUILD ATTEMPTS
#   1) Encoding: this file must stay pure ASCII (no Cyrillic). Saved as
#      UTF-8 with Russian comments, rpmbuild read it with the wrong
#      codepage and failed with
#      "Name field must be present in package ... can't parse".
#   2) Directives inside comments: rpmbuild expands them anyway and
#      warns "Macro expanded in comment". So no percent-sign tokens are
#      written in the comments above.
# ---------------------------------------------------------------

%define debug_package %{nil}

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
Console push-to-talk radio client prototype for Aurora OS: TCP
signalling connection, channel join, PTT talk and listen.
Training project, stages 1-2.

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
