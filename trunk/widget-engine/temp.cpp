        //for pe
	/*    //So lets do a high level reverse engg.
    if(isKicker) {
	//PE_PanelTabWidget is imho what we want
	std::ofstream tempstream("/tmp/kicker",std::ios::app);
	tempstream << pe<<std::endl;
	}*/

	if(isKicker){
		QColor c(cg.button());
        	if(optionHandler->usePanelCustomColor() && optionHandler->panelCustomColor().isValid())
			c = optionHandler->panelCustomColor();
		if(optionHandler->usePanelStipple()){
			p->setPen(c.dark(100+optionHandler->stippleContrast()));
			for(int i=r.top(); i < r.bottom(); i+=4){
				p->drawLine(r.left(), i, r.right(), i);
				p->drawLine(r.left(), i+1, r.right(), i+1);
			}
		}
		else {
			p->fillRect(r,c);
		}
		return;
	}
 
