 $(document).ready(function(){

     $('#m_menu').click(
     function() {
         $('#m_drop').slideToggle(200);
     });

    var deviceAgent = navigator.userAgent.toLowerCase();
    var agentID = deviceAgent.match(/(iphone|ipod|ipad|android)/); 
    var rtn = true;
        
    if(null != agentID) {
        $('ul.nav > li.dropdown > a').click(
            function() {

            $(this).toggleClass('clk');
            $sexl = $(this);
            $oth = $('ul.nav li.dropdown a');
            $oth.each(function(index) {
                $aoth = $(this);
                if($sexl.text() != $aoth.text()) {
                    $aoth.removeClass('clk');
            rtn = false;
                }
            });
            
            var ct = $(this).attr('class');
            if(ct == "") {
                rtn = true;
            } else {
                rtn = false;
            }

        return rtn;
            });
    }
});