
$.fn.mathjaxOuterHTML = function (s) {
  return (s) ? this.before(s).remove() : jQuery("<p>").append(this.eq(0).clone()).html();
};

Drupal.wysiwyg.plugins.mathjax = {
  /**
  * Return whether the passed node belongs to this plugin.
  */
  isNode: function (node) {
    $node = this.getRepresentitiveNode(node);
    return $node.is('div');
  },

  /* We need this due all the special cases in the editors */
  getRepresentitiveNode: function(node) {
    if (node != null && typeof(node) != 'undefined' && '$' in node) {
      // This case is for the CKeditor, where
      // $(node) != $(node.$)
      return $(node.$).parents('div.wysiwyg_mathjax');
    }
    // else
    // This would be for the TinyMCE and hopefully others
    $innerNode = $(node);
    return $innerNode.parents('div.wysiwyg_mathjax');
  },

  /**
  * Execute the button.
  */
  invoke: function (data, settings, instanceId) {
    if(data == null && typeof(data) == 'undefined') {
      return;
    }

    if (data.format == 'html') {
      // Default
      var options = {
        id: instanceId,
        action: 'insert',
        // we need that for changing the mode in the edit form to create a new iid if revisioned
        revisions: Drupal.settings.wysiwyg_mathjax.revisions
      };
      var $node = null;
      if ('node' in data) {
        $node = this.getRepresentitiveNode(data.node);
      }

      if ($node != null && $node.is('div') && $node.hasClass('wysiwyg_mathjax')) {
        $n = $(data.node);
      //  options.iid = decodeURIComponent(data.node.getAttribute('alt'));
        options.iid = $node.attr('id');
        options.action = 'update';
      }
    }
    else {
      // @todo Plain text support.
    }
    // Add or update.
    if (options.action == 'insert') {
      Drupal.wysiwyg.plugins.mathjax.add_form(data, settings, instanceId);
    }
    else if (options.action == 'update') {
      // $node.remove();
      Drupal.wysiwyg.plugins.mathjax.update_form(data, settings, instanceId, options);
    }
  },

  /*
  * Open a dialog and present the add-formula form.
  */
  add_form: function (data, settings, instanceId) {
    // Create the buttons
    var dialogIframe = Drupal.jqui_dialog.iframeSelector();
    var btns = {};
    btns[Drupal.t('Insert')] = function () {
      // well lets test if a formula has been selected
      var formname = 'form#wysiwyg-mathjax-edit-form';
      var formsize = $(dialogIframe).contents().find(formname).size();
      if (formsize == 0) {
      // we clicked 'Insert' on the first screen, which means either we did not
      // select a file (which immediately goes to the edit form), or we did or
      // did not paste MathML into the text box
        var mytext = $(dialogIframe).contents().find('#edit-wysiwyg-mathml-text');
        if ($(mytext).val() == undefined || $(mytext).val().length == 0) {
          alert(Drupal.t("Please first select a formula to upload, or paste your MathML content into the text box."));
          return;
        }
        $('form#wysiwyg-mathjax-upload-form').submit(); // we were on the first form
        return;
      }
      // else
      var iid = 0;
      var form = $(dialogIframe).contents().find(formname);
      form.ajaxSubmit({
        dataType : 'json',
        method: 'post',
        async: false,
        data: { 
        	submitimagedetails : 'JSinsert', 
        	parent_build_id: Drupal.settings.wysiwyg_mathjax.current_form 
        },
        success : function(data,status,xhr,jq) {
            iid = data.data.iid;
            if(!iid) {
              return;
            }
          }
        }
      );
      Drupal.wysiwyg.plugins.mathjax.createFormulaInContent(iid,instanceId);
      $(this).dialog("close");
    };

    btns[Drupal.t('Cancel')] = function () {
      $(this).dialog("close");
    };
    var parent_build_id = Drupal.settings.wysiwyg_mathjax.current_form;
    // Location, where to fetch the dialog.
    var aurl = Drupal.settings.basePath + 'index.php?q=wysiwyg_mathjax/upload/' + parent_build_id;
    // Open the dialog, load the form.
    Drupal.jqui_dialog.open({
      url: aurl,
      buttons: btns,
      width: 540,
      namespace: 'wysiwyg_mathjax'
    });
  },

  /*
  * Open a image-details dialog, prefilled with the current settings of the
  * selected image.
  */
  update_form: function (data, settings, instanceId, options) {
    // Create buttons.
    var dialogIframe = Drupal.jqui_dialog.iframeSelector();
    var btns = {};
    // Update button.
    btns[Drupal.t('Update')] = function () {
      var iid = 0;
      $(dialogIframe).contents().find('form#wysiwyg-mathjax-edit-form').ajaxSubmit({
        dataType : 'json',
        method: 'post',
        data: { 
        	revisions: options.revisions, 
        	parent_build_id: Drupal.settings.wysiwyg_mathjax.current_form, 
        	submitimagedetails : 'JSinsert' 
		},
        async: false,
        success : function(data,status,xhr,jq) {
            iid = data.data.iid;
            if(!iid) {
              return;
            }
          }
        }
      );
      $node.remove();
      Drupal.wysiwyg.plugins.mathjax.createFormulaInContent(iid,instanceId);
      $(this).dialog("close");
    };
    // Cancel button
    btns[Drupal.t('Cancel')] = function () {
      $(this).dialog("close");
    };

    // Location, where to fetch the dialog.
    var aurl = Drupal.settings.basePath + 'index.php?q=wysiwyg_mathjax/edit/' + options.iid +'/' + Drupal.settings.wysiwyg_mathjax.current_form;
    // Finally open the dialog.
    Drupal.jqui_dialog.open({
      url: aurl,
      buttons: btns,
      width: 540,
      namespace: 'wysiwyg_mathjax'
    });
  },

  /*
  * Fetches the formula and insert it all th way down into the current editor
  */
  createFormulaInContent: function (iid, editor_id) {
    Drupal.wysiwyg.plugins.mathjax.insertIntoEditor(this.get_rendered_wysiwyg_formula(iid), editor_id);
  },

  /*
  * Thats the most critical part. Call the WYSIWYG API to insert this html into
  * the current editor, no matter what editor it might be
  */
  insertIntoEditor: function (data, editor_id) {
    // This is all the magic
    Drupal.wysiwyg.instances[editor_id].insert(data);
  },

  attach: function(content, pluginSettings, id) {
    var plugin = this;
    var iids = [];
    content = content.replace(/\[\[wysiwyg_mathjax:(\d+):([^\]]*?)\]\]/g, function(orig, match) {
      iids.push(match);
      return orig;
    });

    if (iids.length === 0) {
      return content;
    }

    iids = plugin.uniqueArray(iids);
    var formulae = plugin.get_rendered_wysiwyg_formulae(iids);

    content = content.replace(
      /\[\[wysiwyg_mathjax:(\d+):([^\]]*?)\]\]/g,
      function(orig, iid, attributes) {
        // Render arguments.
        attributes = attributes.split(',');
        for (var i=0; i<attributes.length; i++) {
            var attribute = attributes[i].split('=');
            attributes[i] = attribute[0] + '="' + attribute[1] + '"';
        }
        return formulae[iid].replace('wysiwyg_placeholder="1"', attributes.join(' '));
      }
    );
    return content;
  },

  detach: function (content, pluginSettings, id)  {
    var plugin = this;
    content = '<div>'+content+'</div>';
    var $content = $(content);
    $content.find('div.wysiwyg_mathjax').map(
      function(i, math) {
        var $math = $(math);
        // Thats the inlineID we use for extracting the meta data from the database
        var inlineID = $math.attr('id');
        
        var attributes = plugin.get_inline_attributes($math);
        var inlineAttribs = attributes.join(',');
        $(math, $content).replaceWith('[[wysiwyg_mathjax:'+inlineID+':' + inlineAttribs + ']]');
      }
    );
    content = $content.html();
    $content.remove();
   
    return content;
  },

  get_rendered_wysiwyg_formula: function(iid) {
      var result = '';
      $.ajax( {
        url: Drupal.settings.basePath + 'index.php?q=ajax/wysiwyg_mathjax/render_wysiwyg/' + iid,
        async: false,
        success: function (data, status) {
          result = $(data.data).mathjaxOuterHTML();
        },
        dataType: 'json'
      }
    );
    return result;
  },

  get_rendered_wysiwyg_formulae: function(iids) {
    var result = [];
    $.ajax( {
        url: Drupal.settings.basePath + 'index.php?q=ajax/wysiwyg_mathjax/render_wysiwyg_formulae/' + iids.join(','),
        async: false,
        success: function (data, status) {
          result = data.data;
        },
        dataType: 'json'
      }
    );
    return result;
  },
  
  get_inline_attributes: function($img) {
    var attributes = Array();
    
    var height = parseInt($img.css('height'), 10);
    if(height) {
      attributes.push('height=' + height);
    }
    
    var width = parseInt($img.css('width'), 10);
    if(width) {
      attributes.push('width=' + width);
    }
    return attributes;
  },
  
  uniqueArray: function (a) {
    var r = new Array();
    o: for (var i = 0, n = a.length; i < n; i++) {
        for (var x = 0, y = r.length; x < y; x++) {
            if (r[x] == a[i])
                continue o;
        }
        r[r.length] = a[i];
    }
    return r;
  }
};
