// pages/library/library.js
Page({

  /**
   * 页面的初始数据
   */
  data: {
    title:"图书管理系统",
    searchname:"",
    searchlocation:"",
    /* 图书信息 */
		booksList: [
        //图书列表
        //图 名字 位置
        {
          img: "/images/book2.jpeg",
          name: "书籍1",
          location: "1区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍2",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍3",
          location: "3区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍4",
          location: "4区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍5",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍6",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍7",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍8",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍9",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍10",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍11",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍12",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍13",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍14",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍15",
          location: "2区",
          islocation:true,
          default:"2区",
        },
        {
          img: "/images/book2.jpeg",
          name: "书籍16",
          location: "2区",
          islocation:true,
          default:"2区",
        },
    ],
  },

  // 搜索图书
  onSearch(e) {
    let that = this;
    for (var i = 0; i < 16; ++i) {
      if(that.data.booksList[i].name == e.detail)
      {
        var newData = {};
        newData['searchname'] = that.data.booksList[i].name;
        newData['searchlocation'] = that.data.booksList[i].location;
        that.setData(newData);
      }
    }
    console.log(e.detail)
  },

})