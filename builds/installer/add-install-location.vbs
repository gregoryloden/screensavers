Dim database : Set database = Wscript.CreateObject("WindowsInstaller.Installer").OpenDatabase("Release/installer.msi", 1)
database.OpenView("INSERT INTO `Property` (`Property`, `Value`) VALUES ('ARPINSTALLLOCATION', '" + Wscript.Arguments(0) + "')").Execute
database.Commit
