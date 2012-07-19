---
layout: default
title: Installing ibus-avro on Fedora 17
---

###Installing ibus-avro on Fedora 17

Open **Terminal** and enter the following commands one by one.

Step 1: Become root

	sudo -s
or
	su 
	
Step 2: Add ibus-avro repository

	cd /etc/yum.repos.d/
	wget http://download.opensuse.org/repositories/home:sarimkhan/Fedora_17/home:sarimkhan.repo

Step 3: Install ibus-avro

	yum install ibus-avro
	

###Setting IBus and Avro

You may need to set IBus as your default input method so that it starts automatically every time you log on.

 1. Open _Activity_ by moving your mouse to top-left corner of your screen.
 2. Open __Input Method Selector__ by typing its name or navigating from _Applications_
 3. Click __Use IBus__ and a second as ibus starts. 
 4. Click `Preferences` beside __Use IBus__.
 5. Go to `Input method`
 6. Click `Select an input method -> Show All Input Methods`
 7. Full list of input method will be shown, click `Bengali -> Avro Phonetic`
 8. Now Click `Add` button to add __Avro Phonetic__ to the list
 9. Now Close `Preferences` and Press `Log Out` from __Input Method Selector__ window.
 10. You will be logged out, Log in again and Open any writing app Ex:Gedit
 11. Now Press `Ctrl+Space` to toggle between _English_ and _Avro_ (Bengali)
 12. Enjoy __Avro Phonetic!__
 
###Uninstalling ibus-avro

	yum remove ibus-avro